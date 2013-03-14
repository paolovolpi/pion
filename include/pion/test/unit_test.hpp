// ---------------------------------------------------------------------
// pion:  a Boost C++ framework for building lightweight HTTP interfaces
// ---------------------------------------------------------------------
// Copyright (C) 2007-2012 Cloudmeter, Inc.  (http://www.cloudmeter.com)
//
// Distributed under the Boost Software License, Version 1.0.
// See http://www.boost.org/LICENSE_1_0.txt
//

#ifndef __PION_TEST_UNIT_TEST_HEADER__
#define __PION_TEST_UNIT_TEST_HEADER__

#include <iostream>
#include <fstream>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_log.hpp>
#include <boost/test/output/xml_log_formatter.hpp>
#include <boost/test/test_case_template.hpp>
#include <pion/logger.hpp>

#ifdef _MSC_VER
    #include <direct.h>
    #define CHANGE_DIRECTORY _chdir
    #define GET_DIRECTORY(a,b) _getcwd(a,b)
#else
    #include <unistd.h>
    #define CHANGE_DIRECTORY chdir
    #define GET_DIRECTORY(a,b) getcwd(a,b)
#endif

#define DIRECTORY_MAX_SIZE 1000


namespace pion {    // begin namespace pion
namespace test {    // begin namespace test
    
    /// thread-safe wrapper for Boost.Tests's XML log formatter
    class safe_xml_log_formatter
        : public boost::unit_test::output::xml_log_formatter
    {
    public:
        
        /// default constructor
        safe_xml_log_formatter()
            : m_entry_in_progress(false)
        {}
        
        /// virtual destructor
        virtual ~safe_xml_log_formatter() {}
    
        /// wrapper to flush output for xml_log_formatter::log_start
        virtual void log_start(std::ostream& ostr,
                               boost::unit_test::counter_t test_cases_amount )
        {
            xml_log_formatter::log_start(ostr, test_cases_amount);
            ostr << std::endl;
        }
    
        /// wrapper to flush output for xml_log_formatter::log_finish
        virtual void log_finish(std::ostream& ostr)
        {
            xml_log_formatter::log_finish(ostr);
            ostr << std::endl;
        }
    
        /// wrapper to flush output for xml_log_formatter::log_build_info
        virtual void log_build_info(std::ostream& ostr)
        {
            xml_log_formatter::log_build_info(ostr);
            ostr << std::endl;
        }
    
        /// wrapper to flush output for xml_log_formatter::test_unit_start
        virtual void test_unit_start(std::ostream& ostr,
                                     boost::unit_test::test_unit const& tu )
        {
            xml_log_formatter::test_unit_start(ostr, tu);
            ostr << std::endl;
        }
    
        /// wrapper to flush output for xml_log_formatter::test_unit_finish
        virtual void test_unit_finish(std::ostream& ostr,
                                      boost::unit_test::test_unit const& tu,
                                      unsigned long elapsed )
        {
            xml_log_formatter::test_unit_finish(ostr, tu, elapsed);
            ostr << std::endl;
        }
    
        /// wrapper to flush output for xml_log_formatter::test_unit_skipped
        virtual void test_unit_skipped(std::ostream& ostr,
                                       boost::unit_test::test_unit const& tu )
        {
            xml_log_formatter::test_unit_skipped(ostr, tu);
            ostr << std::endl;
        }
    
        /// wrapper to flush output for xml_log_formatter::log_exception
        virtual void log_exception(std::ostream& ostr,
                                   boost::unit_test::log_checkpoint_data const& d,
                                   boost::execution_exception const& ex )
        {
            xml_log_formatter::log_exception(ostr, d, ex);
            ostr << std::endl;
        }
    
        /// thread-safe wrapper for xml_log_formatter::log_entry_start
        virtual void log_entry_start( std::ostream& ostr,
                                     boost::unit_test::log_entry_data const& entry_data,
                                     log_entry_types let )
        {
            boost::mutex::scoped_lock entry_lock(m_mutex);
            while (m_entry_in_progress) {
                m_entry_complete.wait(entry_lock);
            }
            m_entry_in_progress = true;
            xml_log_formatter::log_entry_start(ostr, entry_data, let);
            ostr.flush();
        }
    
        /// thread-safe wrapper for xml_log_formatter::log_entry_value
        /// ensures that an entry is in progress
        virtual void log_entry_value( std::ostream& ostr, boost::unit_test::const_string value )
        {
            boost::mutex::scoped_lock entry_lock(m_mutex);
            if (m_entry_in_progress) {
                xml_log_formatter::log_entry_value(ostr, value);
                ostr.flush();
            }
        }
        
        /// thread-safe wrapper for xml_log_formatter::log_entry_finish
        /// assumes the current thread has control via call to log_entry_start()
        virtual void log_entry_finish( std::ostream& ostr )
        {
            boost::mutex::scoped_lock entry_lock(m_mutex);
            if (m_entry_in_progress) {
                xml_log_formatter::log_entry_finish(ostr);
                ostr << std::endl;
                m_entry_in_progress = false;
                m_entry_complete.notify_one();
            }
        }
        
    private:
        
        /// true if a log entry is in progress
        volatile bool       m_entry_in_progress;
        
        /// condition used to signal the completion of a log entry
        boost::condition    m_entry_complete;
        
        /// mutex used to prevent multiple threads from interleaving entries
        boost::mutex        m_mutex;
    };
    
    
    /// config is intended for use as a global fixture.  By including the 
    /// following line in one source code file of a unit test project, the constructor will
    /// run once before the first test and the destructor will run once after the last test:
    ///
    /// BOOST_GLOBAL_FIXTURE(pion::test::config);
    struct config {
        config() {
            std::cout << "global setup for all pion unit tests\n";
            
            // argc and argv do not include parameters handled by the boost unit test framework, such as --log_level.
            int argc = boost::unit_test::framework::master_test_suite().argc;
            char** argv = boost::unit_test::framework::master_test_suite().argv;
            bool verbose = false;

            if (argc > 1) {
                if (argv[1][0] == '-' && argv[1][1] == 'v') {
                    verbose = true;
                } else if (strlen(argv[1]) > 13 && strncmp(argv[1], "--log_output=", 13) == 0) {
                    const char * const test_log_filename = argv[1] + 13;
                    m_test_log_file.open(test_log_filename);
                    if (m_test_log_file.is_open()) {
                        boost::unit_test::unit_test_log.set_stream(m_test_log_file);
                        boost::unit_test::unit_test_log.set_formatter(new safe_xml_log_formatter);
                    } else {
                        std::cerr << "unable to open " << test_log_filename << std::endl;
                    }
                }
            }
    
            if (verbose) {
                PION_LOG_CONFIG_BASIC;
            } else {
                std::cout << "Use '-v' to enable logging of errors and warnings from pion.\n";
            }

            pion::logger log_ptr = PION_GET_LOGGER("pion");
            PION_LOG_SETLEVEL_WARN(log_ptr);
        }
        virtual ~config() {
            std::cout << "global teardown for all pion unit tests\n";
        }

        /// xml log results output stream (needs to be global)
        static std::ofstream    m_test_log_file;
    };
    

    // removes line endings from a c-style string
    static inline char* trim(char* str) {
        for (long len = strlen(str) - 1; len >= 0; len--) {
            if (str[len] == '\n' || str[len] == '\r')
                str[len] = '\0';
            else
                break;
        }
        return str;
    }

    // reads lines from a file, stripping line endings and ignoring blank lines
    // and comment lines (starting with a '#')
    static inline bool read_lines_from_file(const std::string& filename, std::list<std::string>& lines) {
        // open file
        std::ifstream a_file(filename.c_str(), std::ios::in | std::ios::binary);
        if (! a_file.is_open())
            return false;

        // read data from file
        static const unsigned int BUF_SIZE = 4096;
        char *ptr, buf[BUF_SIZE+1];
        buf[BUF_SIZE] = '\0';
        lines.clear();

        while (a_file.getline(buf, BUF_SIZE)) {
            ptr = trim(buf);
            if (*ptr != '\0' && *ptr != '#')
                lines.push_back(ptr);
        }

        // close file
        a_file.close();

        return true;
    }

    // Check for file match, use std::list for sorting the files, which will allow
    // random order matching...
    static inline bool check_files_match(const std::string& fileA, const std::string& fileB) {
        // open and read data from files
        std::list<std::string> a_lines, b_lines;
        BOOST_REQUIRE(read_lines_from_file(fileA, a_lines));
        BOOST_REQUIRE(read_lines_from_file(fileB, b_lines));

        // sort lines read
        a_lines.sort();
        b_lines.sort();

        // files match if lines match
        return (a_lines == b_lines);
    }

    static inline bool check_files_exact_match(const std::string& fileA, const std::string& fileB, bool ignore_line_endings = false) {
        // open files
        std::ifstream a_file(fileA.c_str(), std::ios::in | std::ios::binary);
        BOOST_REQUIRE(a_file.is_open());

        std::ifstream b_file(fileB.c_str(), std::ios::in | std::ios::binary);
        BOOST_REQUIRE(b_file.is_open());

        // read and compare data in files
        static const unsigned int BUF_SIZE = 4096;
        char a_buf[BUF_SIZE];
        char b_buf[BUF_SIZE];

        if (ignore_line_endings) {
            while (a_file.getline(a_buf, BUF_SIZE)) {
                if (! b_file.getline(b_buf, BUF_SIZE))
                    return false;
                trim(a_buf);
                trim(b_buf);
                if (strlen(a_buf) != strlen(b_buf))
                    return false;
                if (memcmp(a_buf, b_buf, strlen(a_buf)) != 0)
                    return false;
            }
            if (b_file.getline(b_buf, BUF_SIZE))
                return false;
        } else {
            while (a_file.read(a_buf, BUF_SIZE)) {
                if (! b_file.read(b_buf, BUF_SIZE))
                    return false;
                if (memcmp(a_buf, b_buf, BUF_SIZE) != 0)
                    return false;
            }
            if (b_file.read(b_buf, BUF_SIZE))
                return false;
        }
        if (a_file.gcount() != b_file.gcount())
            return false;
        if (memcmp(a_buf, b_buf, a_file.gcount()) != 0)
            return false;

        a_file.close();
        b_file.close();

        // files match
        return true;
    }


}   // end namespace test
}   // end namespace pion


/*
Using BOOST_AUTO_TEST_SUITE_FIXTURE_TEMPLATE and
BOOST_AUTO_TEST_CASE_FIXTURE_TEMPLATE has two additional benefits relative to 
using BOOST_FIXTURE_TEST_SUITE and BOOST_AUTO_TEST_CASE:
1) it allows a test to be run with more than one fixture, and
2) it makes the current fixture part of the test name, e.g. 
   checkPropertyX<myFixture_F>

For an example of 1), see http_message_tests.cpp.

There are probably simpler ways to achieve 2), but since it comes for free,
it makes sense to use it.  The benefit of this is that the test names don't
have to include redundant information about the fixture, e.g. 
checkMyFixtureHasPropertyX.  (In this example, checkPropertyX<myFixture_F> is 
not obviously better than checkMyFixtureHasPropertyX, but in many cases the 
test names become too long and/or hard to parse, or the fixture just isn't
part of the name, making some error reports ambiguous.)

(BOOST_AUTO_TEST_CASE_FIXTURE_TEMPLATE is based on BOOST_AUTO_TEST_CASE_TEMPLATE,
in unit_test_suite.hpp.)


Minimal example demonstrating usage of BOOST_AUTO_TEST_CASE_FIXTURE_TEMPLATE:

class ObjectToTest_F { // suffix _F is used for fixtures
public:
    ObjectToTest_F() {
        m_value = 2;
    }
    int m_value;
    int get_value() { return m_value; }
};

// This illustrates the most common case, where just one fixture will be used,
// so the list only has one fixture in it.
// ObjectToTest_S is the name of the test suite.
BOOST_AUTO_TEST_SUITE_FIXTURE_TEMPLATE(ObjectToTest_S,
                                       boost::mpl::list<ObjectToTest_F>)

// One method for testing the fixture...
BOOST_AUTO_TEST_CASE_FIXTURE_TEMPLATE(checkValueEqualsTwo) {
    BOOST_CHECK_EQUAL(F::m_value, 2);
    BOOST_CHECK_EQUAL(F::get_value(), 2);
}

// Another method for testing the fixture...
BOOST_AUTO_TEST_CASE_FIXTURE_TEMPLATE(checkValueEqualsTwoAgain) {
    BOOST_CHECK_EQUAL(this->m_value, 2);
    BOOST_CHECK_EQUAL(this->get_value(), 2);
}

// The simplest, but, alas, non conformant to the C++ standard, method for testing the fixture.
// This will compile with MSVC (unless language extensions are disabled (/Za)).
// It won't compile with gcc unless -fpermissive is used.
// See http://gcc.gnu.org/onlinedocs/gcc/Name-lookup.html.
BOOST_AUTO_TEST_CASE_FIXTURE_TEMPLATE(checkValueEqualsTwoNonConformant) {
    BOOST_CHECK_EQUAL(m_value, 2);
    BOOST_CHECK_EQUAL(get_value(), 2);
}

BOOST_AUTO_TEST_SUITE_END()
*/

#define BOOST_AUTO_TEST_SUITE_FIXTURE_TEMPLATE(suite_name, fixture_types) \
BOOST_AUTO_TEST_SUITE(suite_name)                                         \
typedef fixture_types BOOST_AUTO_TEST_CASE_FIXTURE_TYPES;                 \
/**/

#define BOOST_AUTO_TEST_CASE_FIXTURE_TEMPLATE(test_name)        \
template<typename F>                                            \
struct test_name : public F                                     \
{ void test_method(); };                                        \
                                                                \
struct BOOST_AUTO_TC_INVOKER( test_name ) {                     \
    template<typename TestType>                                 \
    static void run( boost::type<TestType>* = 0 )               \
    {                                                           \
        test_name<TestType> t;                                  \
        t.test_method();                                        \
    }                                                           \
};                                                              \
                                                                \
BOOST_AUTO_TU_REGISTRAR( test_name )(                           \
    boost::unit_test::ut_detail::template_test_case_gen<        \
        BOOST_AUTO_TC_INVOKER( test_name ),                     \
        BOOST_AUTO_TEST_CASE_FIXTURE_TYPES >(                   \
            BOOST_STRINGIZE( test_name ) ) );                   \
                                                                \
template<typename F>                                            \
void test_name<F>::test_method()                                \
/**/


#endif
