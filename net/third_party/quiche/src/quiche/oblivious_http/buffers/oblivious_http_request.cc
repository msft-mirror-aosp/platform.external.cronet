#include "quiche/oblivious_http/buffers/oblivious_http_request.h"

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <string>
#include <utility>

#include "absl/memory/memory.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "openssl/hpke.h"
#include "quiche/common/platform/api/quiche_bug_tracker.h"
#include "quiche/common/platform/api/quiche_logging.h"
#include "quiche/common/quiche_crypto_logging.h"

namespace quiche {
// Ctor.
ObliviousHttpRequest::Context::Context(
    bssl::UniquePtr<EVP_HPKE_CTX> hpke_context, std::string encapsulated_key)
    : hpke_context_(std::move(hpke_context)),
      encapsulated_key_(std::move(encapsulated_key)) {}

// Ctor.
ObliviousHttpRequest::ObliviousHttpRequest(
    bssl::UniquePtr<EVP_HPKE_CTX> hpke_context, std::string encapsulated_key,
    const ObliviousHttpHeaderKeyConfig& ohttp_key_config,
    std::string req_ciphertext, std::string req_plaintext)
    : oblivious_http_request_context_(absl::make_optional(
          Context(std::move(hpke_context), std::move(encapsulated_key)))),
      key_config_(ohttp_key_config),
      request_ciphertext_(std::move(req_ciphertext)),
      request_plaintext_(std::move(req_plaintext)) {}

// Request Decapsulation.
absl::StatusOr<ObliviousHttpRequest>
ObliviousHttpRequest::CreateServerObliviousRequest(
    absl::string_view encrypted_data, const EVP_HPKE_KEY& gateway_key,
    const ObliviousHttpHeaderKeyConfig& ohttp_key_config) {
  if (EVP_HPKE_KEY_kem(&gateway_key) == nullptr) {
    return absl::InvalidArgumentError(
        "Invalid input param. Failed to import gateway_key.");
  }
  bssl::UniquePtr<EVP_HPKE_CTX> gateway_ctx(EVP_HPKE_CTX_new());
  if (gateway_ctx == nullptr) {
    return SslErrorAsStatus("Failed to initialize Gateway/Server's Context.");
  }
  // TODO(anov) Add ParseOhttpPayloadHeader(QuicheDataReader) to read fields out
  // of payload, and eliminate sub-stringing.
  auto is_hdr_ok = ohttp_key_config.ParseOhttpPayloadHeader(encrypted_data);
  if (!is_hdr_ok.ok()) {
    return is_hdr_ok;
  }
  absl::string_view enc_plus_ciphertext =
      encrypted_data.substr(ObliviousHttpHeaderKeyConfig::kHeaderLength);

  size_t enc_key_len = EVP_HPKE_KEM_enc_len(EVP_HPKE_KEY_kem(&gateway_key));
  if (enc_plus_ciphertext.size() < enc_key_len) {
    return absl::FailedPreconditionError(absl::StrCat(
        "Failed to extract encapsulation key of expected len=", enc_key_len,
        "from payload."));
  }
  absl::string_view enc_key_received =
      enc_plus_ciphertext.substr(0, enc_key_len);
  std::string info = ohttp_key_config.SerializeRecipientContextInfo();
  if (!EVP_HPKE_CTX_setup_recipient(
          gateway_ctx.get(), &gateway_key, ohttp_key_config.GetHpkeKdf(),
          ohttp_key_config.GetHpkeAead(),
          reinterpret_cast<const uint8_t*>(enc_key_received.data()),
          enc_key_received.size(),
          reinterpret_cast<const uint8_t*>(info.data()), info.size())) {
    return SslErrorAsStatus("Failed to setup recipient context");
  }

  absl::string_view ciphertext_received =
      enc_plus_ciphertext.substr(enc_key_len);

  // Decrypt the message.
  std::string decrypted(ciphertext_received.size(), '\0');
  size_t decrypted_len;
  if (!EVP_HPKE_CTX_open(
          gateway_ctx.get(), reinterpret_cast<uint8_t*>(decrypted.data()),
          &decrypted_len, decrypted.size(),
          reinterpret_cast<const uint8_t*>(ciphertext_received.data()),
          ciphertext_received.size(), nullptr, 0)) {
    return SslErrorAsStatus("Failed to decrypt.");
  }
  decrypted.resize(decrypted_len);
  return ObliviousHttpRequest(
      std::move(gateway_ctx), std::string(enc_key_received), ohttp_key_config,
      std::string(ciphertext_received), std::move(decrypted));
}

// Request Encapsulation.
absl::StatusOr<ObliviousHttpRequest>
ObliviousHttpRequest::CreateClientObliviousRequest(
    absl::string_view plaintext_payload, absl::string_view hpke_public_key,
    const ObliviousHttpHeaderKeyConfig& ohttp_key_config) {
  return EncapsulateWithSeed(plaintext_payload, hpke_public_key,
                             ohttp_key_config, "");
}

absl::StatusOr<ObliviousHttpRequest>
ObliviousHttpRequest::CreateClientWithSeedForTesting(
    absl::string_view plaintext_payload, absl::string_view hpke_public_key,
    const ObliviousHttpHeaderKeyConfig& ohttp_key_config,
    absl::string_view seed) {
  return ObliviousHttpRequest::EncapsulateWithSeed(
      plaintext_payload, hpke_public_key, ohttp_key_config, seed);
}

absl::StatusOr<ObliviousHttpRequest> ObliviousHttpRequest::EncapsulateWithSeed(
    absl::string_view plaintext_payload, absl::string_view hpke_public_key,
    const ObliviousHttpHeaderKeyConfig& ohttp_key_config,
    absl::string_view seed) {
  if (plaintext_payload.empty() || hpke_public_key.empty()) {
    return absl::InvalidArgumentError("Invalid input.");
  }
  // Initialize HPKE key and context.
  bssl::UniquePtr<EVP_HPKE_KEY> client_key(EVP_HPKE_KEY_new());
  if (client_key == nullptr) {
    return SslErrorAsStatus("Failed to initialize HPKE Client Key.");
  }
  bssl::UniquePtr<EVP_HPKE_CTX> client_ctx(EVP_HPKE_CTX_new());
  if (client_ctx == nullptr) {
    return SslErrorAsStatus("Failed to initialize HPKE Client Context.");
  }
  // Setup the sender (client)
  std::string encapsulated_key(EVP_HPKE_MAX_ENC_LENGTH, '\0');
  size_t enc_len;
  std::string info = ohttp_key_config.SerializeRecipientContextInfo();
  if (seed.empty()) {
    if (!EVP_HPKE_CTX_setup_sender(
            client_ctx.get(),
            reinterpret_cast<uint8_t*>(encapsulated_key.data()), &enc_len,
            encapsulated_key.size(), ohttp_key_config.GetHpkeKem(),
            ohttp_key_config.GetHpkeKdf(), ohttp_key_config.GetHpkeAead(),
            reinterpret_cast<const uint8_t*>(hpke_public_key.data()),
            hpke_public_key.size(),
            reinterpret_cast<const uint8_t*>(info.data()), info.size())) {
      return SslErrorAsStatus(
          "Failed to setup HPKE context with given public key param "
          "hpke_public_key.");
    }
  } else {
    if (!EVP_HPKE_CTX_setup_sender_with_seed_for_testing(
            client_ctx.get(),
            reinterpret_cast<uint8_t*>(encapsulated_key.data()), &enc_len,
            encapsulated_key.size(), ohttp_key_config.GetHpkeKem(),
            ohttp_key_config.GetHpkeKdf(), ohttp_key_config.GetHpkeAead(),
            reinterpret_cast<const uint8_t*>(hpke_public_key.data()),
            hpke_public_key.size(),
            reinterpret_cast<const uint8_t*>(info.data()), info.size(),
            reinterpret_cast<const uint8_t*>(seed.data()), seed.size())) {
      return SslErrorAsStatus(
          "Failed to setup HPKE context with given public key param "
          "hpke_public_key and seed.");
    }
  }
  encapsulated_key.resize(enc_len);
  std::string ciphertext(
      plaintext_payload.size() + EVP_HPKE_CTX_max_overhead(client_ctx.get()),
      '\0');
  size_t ciphertext_len;
  if (!EVP_HPKE_CTX_seal(
          client_ctx.get(), reinterpret_cast<uint8_t*>(ciphertext.data()),
          &ciphertext_len, ciphertext.size(),
          reinterpret_cast<const uint8_t*>(plaintext_payload.data()),
          plaintext_payload.size(), nullptr, 0)) {
    return SslErrorAsStatus(
        "Failed to encrypt plaintext_payload with given public key param "
        "hpke_public_key.");
  }
  ciphertext.resize(ciphertext_len);
  if (encapsulated_key.empty() || ciphertext.empty()) {
    return absl::InternalError(absl::StrCat(
        "Failed to generate required data: ",
        (encapsulated_key.empty() ? "encapsulated key is empty" : ""),
        (ciphertext.empty() ? "encrypted data is empty" : ""), "."));
  }

  return ObliviousHttpRequest(
      std::move(client_ctx), std::move(encapsulated_key), ohttp_key_config,
      std::move(ciphertext), std::string(plaintext_payload));
}

// Request Serialize.
// Builds request=[hdr, enc, ct].
// https://www.ietf.org/archive/id/draft-ietf-ohai-ohttp-03.html#section-4.1-4.5
std::string ObliviousHttpRequest::EncapsulateAndSerialize() const {
  if (!oblivious_http_request_context_.has_value()) {
    QUICHE_BUG(ohttp_encapsulate_after_context_extract)
        << "EncapsulateAndSerialize cannot be called after ReleaseContext()";
    return "";
  }
  return absl::StrCat(key_config_.SerializeOhttpPayloadHeader(),
                      oblivious_http_request_context_->encapsulated_key_,
                      request_ciphertext_);
}

// Returns Decrypted blob in the case of server, and returns plaintext used by
// the client while `CreateClientObliviousRequest`.
absl::string_view ObliviousHttpRequest::GetPlaintextData() const {
  return request_plaintext_;
}

}  // namespace quiche
