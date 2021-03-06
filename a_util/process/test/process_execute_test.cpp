/**
 * @file
 * Thread test implementation
 *
 * @copyright
 * @verbatim
   Copyright @ 2017 Audi Electronics Venture GmbH. All rights reserved.

       This Source Code Form is subject to the terms of the Mozilla
       Public License, v. 2.0. If a copy of the MPL was not distributed
       with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

   If it is not possible or desirable to put the notice in a particular file, then
   You may include the notice in a location (such as a LICENSE file in a
   relevant directory) where a recipient would be likely to look for such a notice.

   You may add additional accurate notices of copyright ownership.
   @endverbatim
*/

#include <cstddef>
#include "gtest/gtest.h"
#include "a_util/process.h"

using namespace a_util;

void teardown_test()
{
    a_util::filesystem::Path dir_bat = a_util::filesystem::getTempDirectory();
    dir_bat.append("dir.bat");
    a_util::filesystem::remove(dir_bat);
}

a_util::filesystem::Path prepare_test()
{
#ifdef WIN32
    teardown_test();

    a_util::filesystem::Path dir_bat = a_util::filesystem::getTempDirectory();
    dir_bat.append("dir.bat");
    a_util::filesystem::writeTextFile(dir_bat, "dir\n");
    return dir_bat;
#else
    return a_util::filesystem::Path("ls");
#endif // WIN32
}

TEST(process_test, TestExecute_No_Wait)
{
#ifdef WIN32
    a_util::filesystem::Path the_call_cmd = "NETSTAT.EXE";
#else
    a_util::filesystem::Path the_call_cmd = "ls";
#endif
    // POS Tests
    // start the ls/dir and do not wait ... it will NOT fail
    ASSERT_EQ(
        a_util::process::execute(the_call_cmd, "", a_util::filesystem::getTempDirectory(), false),
        0);

    // start the ls/dir and do not wait ... it will not fail because it uses the current dir as
    // working dir usually
    ASSERT_EQ(a_util::process::execute(the_call_cmd, "", "", false), 0);

    // NEG Tests
    // start the ls/dir and do not wait ... will NOT fail also if the commandline arg is not valid
    ASSERT_EQ(a_util::process::execute(the_call_cmd, "--this_is_not_valid", "", false), 0);

    // start the ls/dir and do not wait ... will fail ... the working dir does not exist
    ASSERT_TRUE(a_util::process::execute(the_call_cmd, "", "this_does_not_exist", false) != 0);

    // start the ls/dir and do not wait ... will fail ... the call command daoes not exist
    ASSERT_TRUE(a_util::process::execute("this_does_not_exist", "", "", false) != 0);
}

TEST(process_test, TestExecute_And_Wait)
{
#ifdef WIN32
    a_util::filesystem::Path the_call_cmd = "NETSTAT.EXE";
#else
    a_util::filesystem::Path the_call_cmd = "ls";
#endif
    // POS Tests
    // start the ls/dir and do not wait ... it will NOT fail
    ASSERT_EQ(
        a_util::process::execute(the_call_cmd, "", a_util::filesystem::getTempDirectory(), true),
        0);

    // start the ls/dir and do not wait ... it will not fail because it uses the current dir as
    // working dir usually
    ASSERT_EQ(a_util::process::execute(the_call_cmd, "", "", true), 0);

    // NEG TESTS
    // start the ls/dir and do not wait ... will fail now
    ASSERT_NE(a_util::process::execute(the_call_cmd, "--this_is_not_valid", "", true), 0);

    // start the ls/dir and do not wait ... will fail ... the working dir does not exist
    ASSERT_NE(a_util::process::execute(the_call_cmd, "", "this_does_not_exist", true), 0);

    // start the ls/dir and do not wait ... will fail ... the call command daoes not exist
    ASSERT_NE(a_util::process::execute("this_does_not_exist", "", "", true), 0);
}

TEST(process_test, TestExecute_And_Wait_with_stdout_return_pos)
{
    a_util::filesystem::Path the_call_cmd = prepare_test();
#define this_is_a_test_dir "this_is_a_test_dir"

    {
        std::string std_out;
        ASSERT_NO_THROW(a_util::filesystem::createDirectory(
            a_util::filesystem::getTempDirectory().append(this_is_a_test_dir)));

        // POS Tests ... we only check the std out once!!
        // start the ls/dir and do not wait ... it will NOT fail
        ASSERT_EQ(a_util::process::execute(
                      the_call_cmd, ".", a_util::filesystem::getTempDirectory(), std_out),
                  0);
        ASSERT_TRUE(!std_out.empty());
        ASSERT_TRUE(std_out.find(std::string(this_is_a_test_dir)) != std::string::npos);

        ASSERT_NO_THROW(a_util::filesystem::remove(
            a_util::filesystem::getTempDirectory().append(this_is_a_test_dir)));
        std_out.clear();
    }

    {
        std::string std_out;
        // start the ls/dir and do not wait ... it will not fail because it uses the current dir as
        // working dir usually
        ASSERT_EQ(a_util::process::execute(the_call_cmd, ".", "", std_out), 0);
        ASSERT_TRUE(!std_out.empty());
    }

    teardown_test();
}

TEST(process_test, TestExecute_And_Wait_with_stdout_return_neg)
{
#ifdef WIN32
    a_util::filesystem::Path the_call_cmd = "NETSTAT.EXE";
#else
    a_util::filesystem::Path the_call_cmd = "ls";
#endif

    // NEG TESTS
    {
        std::string std_out;
        // start the ls/dir and do not wait ... it will not fail because it uses the current dir as
        // working dir usually
        ASSERT_NE(a_util::process::execute(the_call_cmd, "--this_is_not_valid", "", std_out), 0);
        ASSERT_TRUE(std_out.empty()); // on standard out often a message is shown
    }

    {
        std::string std_out;
        // start the ls/dir and do not wait ... will fail ... the working dir does not exist
        ASSERT_NE(a_util::process::execute(the_call_cmd, "", "this_does_not_exist", std_out), 0);
        ASSERT_TRUE(std_out.empty());
    }

    {
        std::string std_out;
        // start the ls/dir and do not wait ... will fail ... the call command daoes not exist
        ASSERT_NE(a_util::process::execute("this_does_not_exist", "", "", std_out), 0);
        ASSERT_TRUE(std_out.empty());
    }
}

TEST(process_test, TestExecute_And_Wait_with_stdout_std_error_return_pos)
{
    a_util::filesystem::Path the_call_cmd = prepare_test();
#define this_is_a_test_dir "this_is_a_test_dir"

    {
        std::string std_out;
        std::string std_err;
        ASSERT_NO_THROW(a_util::filesystem::createDirectory(
            a_util::filesystem::getTempDirectory().append(this_is_a_test_dir)));

        // POS Tests ... we only check the std out once!!
        // start the ls/dir and do not wait ... it will NOT fail
        ASSERT_EQ(a_util::process::execute(
                      the_call_cmd, ".", a_util::filesystem::getTempDirectory(), std_out, std_err),
                  0);
        ASSERT_TRUE(!std_out.empty());
        ASSERT_TRUE(std_out.find(std::string(this_is_a_test_dir)) != std::string::npos);
        ASSERT_TRUE(std_err.empty());

        ASSERT_NO_THROW(a_util::filesystem::remove(
            a_util::filesystem::getTempDirectory().append(this_is_a_test_dir)));
    }

    {
        std::string std_out;
        std::string std_err;
        // start the ls/dir and do not wait ... it will not fail because it uses the current dir as
        // working dir usually
        ASSERT_EQ(a_util::process::execute(the_call_cmd, ".", "", std_out, std_err), 0);
        ASSERT_TRUE(!std_out.empty());
        ASSERT_TRUE(std_err.empty());
    }

    teardown_test();
}

TEST(process_test, TestExecute_And_Wait_with_stdout_std_error_return_neg)
{
#ifdef WIN32
    a_util::filesystem::Path the_call_cmd = "NETSTAT.EXE";
#else
    a_util::filesystem::Path the_call_cmd = "ls";
#endif
    // NEG TESTS
    {
        std::string std_out;
        std::string std_err;
        // start the ls/dir and do not wait ... it will not fail because it uses the current dir as
        // working dir usually
        ASSERT_NE(
            a_util::process::execute(the_call_cmd, "--this_is_not_valid", "", std_out, std_err), 0);
        ASSERT_TRUE(std_out.empty());
        // you cannot say anything about stderr here, could be empty or not
    }

    {
        std::string std_out;
        std::string std_err;
        // start the ls/dir and do not wait ... will fail ... the working dir does not exist
        ASSERT_NE(
            a_util::process::execute(the_call_cmd, "", "this_does_not_exist", std_out, std_err), 0);
        ASSERT_TRUE(std_out.empty());
        // you cannot say anything about stderr here, could be empty or not
    }

    {
        std::string std_out;
        std::string std_err;
        // start the ls/dir and do not wait ... will fail ... the call command does not exist
        ASSERT_NE(a_util::process::execute("this_does_not_exist", "", "", std_out, std_err), 0);
        ASSERT_TRUE(std_out.empty());
        // you cannot say anything about stderr here, could be empty or not
    }
    teardown_test();
}
