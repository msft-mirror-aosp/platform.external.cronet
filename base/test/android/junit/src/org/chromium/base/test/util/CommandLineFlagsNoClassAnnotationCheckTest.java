// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.base.test.util;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.annotation.Config;

import org.chromium.base.CommandLine;
import org.chromium.base.test.BaseRobolectricTestRunner;

/**
 * Unit tests for {@link CommandLineFlags} annotations. This is for testing when there is no
 * annotation on the class level.
 */
@RunWith(BaseRobolectricTestRunner.class)
@Config(manifest = Config.NONE)
public class CommandLineFlagsNoClassAnnotationCheckTest {
    @Test
    public void testNoAnnotation() throws Throwable {
        var switches = CommandLine.getInstance().getSwitches();
        Assert.assertTrue("CommandLine switches should be empty: " + switches, switches.isEmpty());
    }

    @Test
    @CommandLineFlags.Add("some-switch")
    public void testAddSwitch_method() throws Throwable {
        Assert.assertTrue("some-switch should be appended",
                CommandLine.getInstance().hasSwitch("some-switch"));
    }

    @Test
    @CommandLineFlags.Add("some-switch")
    @CommandLineFlags.Remove("some-switch")
    public void testAddThenRemoveSwitch_method() throws Throwable {
        Assert.assertTrue(
                "CommandLine switches should be empty after adding and removing the same switch",
                CommandLine.getInstance().getSwitches().isEmpty());
    }

    @Test
    @CommandLineFlags.Remove("some-switch")
    @CommandLineFlags.Add("some-switch")
    public void testRemoveThenAddSwitch_method() throws Throwable {
        // ".Add" rules apply before ".Remove" rules when annotating the same method/class,
        // regardless of the order the annotations are written.
        Assert.assertTrue(
                "CommandLine switches should be empty after removing and adding the same switch",
                CommandLine.getInstance().getSwitches().isEmpty());
    }
}
