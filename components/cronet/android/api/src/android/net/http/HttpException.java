// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package android.net.http;

import java.io.IOException;

/**
 * Base exception passed to {@link UrlRequest.Callback#onFailed UrlRequest.Callback.onFailed()}.
 */
public class HttpException extends IOException {
    /**
     * Constructs an exception that is caused by {@code cause}.
     *
     * @param message explanation of failure.
     * @param cause the cause (which is saved for later retrieval by the {@link
     *         java.io.IOException#getCause getCause()} method). A null value is permitted, and
     *         indicates that the cause is nonexistent or unknown.
     */
    protected HttpException(String message, Throwable cause) {
        super(message, cause);
    }
}
