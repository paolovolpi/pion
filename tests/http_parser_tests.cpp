// ---------------------------------------------------------------------
// pion:  a Boost C++ framework for building lightweight HTTP interfaces
// ---------------------------------------------------------------------
// Copyright (C) 2007-2012 Cloudmeter, Inc.  (http://www.cloudmeter.com)
//
// Distributed under the Boost Software License, Version 1.0.
// See http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/test/unit_test.hpp>
#include <pion/algorithm.hpp>
#include <pion/http/parser.hpp>
#include <pion/http/request.hpp>
#include <pion/http/response.hpp>

#include "http_parser_tests_data.inc"

using namespace pion;

BOOST_AUTO_TEST_CASE(testParseHttpUri)
{
    std::string uri("http://127.0.0.1:80/folder/file.ext?q=uery");
    std::string proto;
    std::string host;
    boost::uint16_t port = 0;
    std::string path;
    std::string query;

    BOOST_CHECK(http::parser::parse_uri(uri, proto, host, port, path, query));
    BOOST_CHECK_EQUAL(proto, "http");
    BOOST_CHECK_EQUAL(host, "127.0.0.1");
    BOOST_CHECK_EQUAL(port, 80);
    BOOST_CHECK_EQUAL(path, "/folder/file.ext");
    BOOST_CHECK_EQUAL(query, "q=uery");

    uri = "http://www.cloudmeter.com/folder/file.ext";

    BOOST_CHECK(http::parser::parse_uri(uri, proto, host, port, path, query));
    BOOST_CHECK_EQUAL(proto, "http");
    BOOST_CHECK_EQUAL(host, "www.cloudmeter.com");
    BOOST_CHECK_EQUAL(port, 80);
    BOOST_CHECK_EQUAL(path, "/folder/file.ext");
    BOOST_CHECK_EQUAL(query, "");
}

BOOST_AUTO_TEST_CASE(testParseSimpleQueryString)
{
    const std::string QUERY_STRING("a=b");
    ihash_multimap params;
    BOOST_REQUIRE(http::parser::parse_url_encoded(params, QUERY_STRING.c_str(), QUERY_STRING.size()));
    BOOST_CHECK_EQUAL(params.size(), 1UL);

    ihash_multimap::const_iterator i = params.find("a");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK_EQUAL(i->second, "b");
}

BOOST_AUTO_TEST_CASE(testParseQueryStringWithMultipleValues)
{
    const std::string QUERY_STRING("test=2&three=%20four%20with%20spaces&five=sixty+two");
    ihash_multimap params;
    BOOST_REQUIRE(http::parser::parse_url_encoded(params, QUERY_STRING));
    BOOST_CHECK_EQUAL(params.size(), 3UL);

    ihash_multimap::const_iterator i = params.find("test");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK_EQUAL(i->second, "2");
    i = params.find("three");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK_EQUAL(i->second, " four with spaces");
    i = params.find("five");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK_EQUAL(i->second, "sixty two");
}

BOOST_AUTO_TEST_CASE(testParseQueryStringWithDoubleAmpersand)
{
    const std::string QUERY_STRING("a=b&&c=d&e");
    ihash_multimap params;
    BOOST_REQUIRE(http::parser::parse_url_encoded(params, QUERY_STRING));
    BOOST_CHECK_EQUAL(params.size(), 3UL);

    ihash_multimap::const_iterator i = params.find("a");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK_EQUAL(i->second, "b");
    i = params.find("c");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK_EQUAL(i->second, "d");
    i = params.find("e");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK(i->second.empty());
}

BOOST_AUTO_TEST_CASE(testParseQueryStringWithEmptyName)
{
    const std::string QUERY_STRING("a=b&=bob&=&c=d&e");
    ihash_multimap params;
    BOOST_REQUIRE(http::parser::parse_url_encoded(params, QUERY_STRING));
    BOOST_CHECK_EQUAL(params.size(), 3UL);

    ihash_multimap::const_iterator i = params.find("a");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK_EQUAL(i->second, "b");
    i = params.find("c");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK_EQUAL(i->second, "d");
    i = params.find("e");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK(i->second.empty());
}

BOOST_AUTO_TEST_CASE(testParseQueryStringWithEmptyValues)
{
    const std::string QUERY_STRING("a=&b&c=");
    ihash_multimap params;
    BOOST_REQUIRE(http::parser::parse_url_encoded(params, QUERY_STRING));
    BOOST_CHECK_EQUAL(params.size(), 3UL);

    ihash_multimap::const_iterator i = params.find("a");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK(i->second.empty());
    i = params.find("b");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK(i->second.empty());
    i = params.find("c");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK(i->second.empty());
}

BOOST_AUTO_TEST_CASE(testParseQueryStringWithTabs)
{
    const std::string QUERY_STRING("promoCode=BOB	");
    ihash_multimap params;
    BOOST_REQUIRE(http::parser::parse_url_encoded(params, QUERY_STRING));
    BOOST_CHECK_EQUAL(params.size(), 1UL);

    ihash_multimap::const_iterator i = params.find("promoCode");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK_EQUAL(i->second, "BOB");
}

BOOST_AUTO_TEST_CASE(testParseMultipartFormData)
{
    const std::string FORM_DATA("------WebKitFormBoundarynqrI4c1BfROrEpu7\r\n"
                                "Content-Disposition: form-data; name=\"field1\"\r\n"
                                "\r\n"
                                "this\r\n"
                                "------WebKitFormBoundarynqrI4c1BfROrEpu7\r\n"
                                "Content-Disposition: form-data; name=\"field2\"\r\n"
                                "\r\n"
                                "is\r\n"
                                "------WebKitFormBoundarynqrI4c1BfROrEpu7\r\n"
                                "Content-Disposition: form-data; name=\"funny$field1\"\r\n"
                                "\r\n"
                                "a\r\n"
                                "------WebKitFormBoundarynqrI4c1BfROrEpu7\r\n"
                                "Content-Disposition: form-data; name=\"skipme\"\r\n"
                                "content-type: application/octet-stream\r\n"
                                "\r\n"
                                "SKIP ME!\r\n"
                                "------WebKitFormBoundarynqrI4c1BfROrEpu7\r\n"
                                "Content-Disposition: form-data; name=\"funny$field2\"\r\n"
                                "\r\n"
                                "funky test!\r\n"
                                "------WebKitFormBoundarynqrI4c1BfROrEpu7--");
    ihash_multimap params;
    BOOST_REQUIRE(http::parser::parse_multipart_form_data(params, "multipart/form-data; boundary=----WebKitFormBoundarynqrI4c1BfROrEpu7", FORM_DATA));
    BOOST_CHECK_EQUAL(params.size(), 4UL);
    ihash_multimap::const_iterator i;

    i = params.find("skipme");
    BOOST_REQUIRE(i == params.end());

    i = params.find("field1");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK_EQUAL(i->second, "this");

    i = params.find("field2");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK_EQUAL(i->second, "is");

    i = params.find("funny$field1");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK_EQUAL(i->second, "a");

    i = params.find("funny$field2");
    BOOST_REQUIRE(i != params.end());
    BOOST_CHECK_EQUAL(i->second, "funky test!");
}

BOOST_AUTO_TEST_CASE(testParseSingleCookieHeader)
{
    std::string cookie_header;
    ihash_multimap cookies;
    ihash_multimap::const_iterator cookie_it;

    cookie_header = "a=b";
    BOOST_REQUIRE(http::parser::parse_cookie_header(cookies, cookie_header, false));
    BOOST_CHECK_EQUAL(cookies.size(), 1UL);

    cookie_it = cookies.find("a");
    BOOST_REQUIRE(cookie_it != cookies.end());
    BOOST_CHECK_EQUAL(cookie_it->second, "b");
}

BOOST_AUTO_TEST_CASE(testParseTwoCookieHeader)
{
    std::string cookie_header;
    ihash_multimap cookies;
    ihash_multimap::const_iterator cookie_it;

    cookie_header = "a=b; Part_Number=\"Rocket_Launcher_0001\";";
    BOOST_REQUIRE(http::parser::parse_cookie_header(cookies, cookie_header, false));
    BOOST_CHECK_EQUAL(cookies.size(), 2UL);

    cookie_it = cookies.find("a");
    BOOST_REQUIRE(cookie_it != cookies.end());
    BOOST_CHECK_EQUAL(cookie_it->second, "b");

    cookie_it = cookies.find("Part_Number");
    BOOST_REQUIRE(cookie_it != cookies.end());
    BOOST_CHECK_EQUAL(cookie_it->second, "Rocket_Launcher_0001");
}

BOOST_AUTO_TEST_CASE(testParseCookieHeaderWithEmptyName)
{
    std::string cookie_header;
    ihash_multimap cookies;
    ihash_multimap::const_iterator cookie_it;

    cookie_header = "a=b; =; =\"001\"; c=d";
    BOOST_REQUIRE(http::parser::parse_cookie_header(cookies, cookie_header, false));
    BOOST_CHECK_EQUAL(cookies.size(), 2UL);

    cookie_it = cookies.find("a");
    BOOST_REQUIRE(cookie_it != cookies.end());
    BOOST_CHECK_EQUAL(cookie_it->second, "b");

    cookie_it = cookies.find("c");
    BOOST_REQUIRE(cookie_it != cookies.end());
    BOOST_CHECK_EQUAL(cookie_it->second, "d");
}

BOOST_AUTO_TEST_CASE(testParseCookieHeaderWithUnquotedSpaces)
{
    std::string cookie_header;
    ihash_multimap cookies;
    ihash_multimap::const_iterator cookie_it;

    cookie_header = "a=a black cat; c = Dec 2, 2010 11:54:30 AM; d = \"dark \"";
    BOOST_REQUIRE(http::parser::parse_cookie_header(cookies, cookie_header, false));
    BOOST_CHECK_EQUAL(cookies.size(), 4UL);

    cookie_it = cookies.find("a");
    BOOST_REQUIRE(cookie_it != cookies.end());
    BOOST_CHECK_EQUAL(cookie_it->second, "a black cat");

    cookie_it = cookies.find("c");
    BOOST_REQUIRE(cookie_it != cookies.end());
    BOOST_CHECK_EQUAL(cookie_it->second, "Dec 2");

    // ideally, this would be parsed as part of c (as intended)
    // but the parser needs to accept , as a cookie separator to conform with v1 and later
    // so for now just not "breaking" is good enough
    cookie_it = cookies.find("201011:54:30AM");
    BOOST_REQUIRE(cookie_it != cookies.end());
    BOOST_CHECK_EQUAL(cookie_it->second, "");

    cookie_it = cookies.find("d");
    BOOST_REQUIRE(cookie_it != cookies.end());
    BOOST_CHECK_EQUAL(cookie_it->second, "dark ");
}

BOOST_AUTO_TEST_CASE(testParseNormalCookieHeader)
{
    std::string cookie_header;
    ihash_multimap cookies;
    ihash_multimap::const_iterator cookie_it;

    cookie_header = "$Version=\"1\"; Part_Number=\"Rocket_Launcher_0001\"; $Path=\"/acme\"";
    BOOST_REQUIRE(http::parser::parse_cookie_header(cookies, cookie_header, false));
    BOOST_CHECK_EQUAL(cookies.size(), 1UL);
    cookie_it = cookies.find("Part_Number");
    BOOST_REQUIRE(cookie_it != cookies.end());
    BOOST_CHECK_EQUAL(cookie_it->second, "Rocket_Launcher_0001");
}

BOOST_AUTO_TEST_CASE(testParseSetCookieHeader)
{
    std::string cookie_header;
    ihash_multimap cookies;
    ihash_multimap::const_iterator cookie_it;

    cookie_header = "Shipping=\"FedEx\"; Version=\"1\"; Path=\"/acme\"";
    BOOST_REQUIRE(http::parser::parse_cookie_header(cookies, cookie_header, true));
    BOOST_CHECK_EQUAL(cookies.size(), 1UL);
    cookie_it = cookies.find("Shipping");
    BOOST_REQUIRE(cookie_it != cookies.end());
    BOOST_CHECK_EQUAL(cookie_it->second, "FedEx");
}

BOOST_AUTO_TEST_CASE(testHTTPParserSimpleRequest)
{
    http::parser request_parser(true);
    request_parser.set_read_buffer((const char*)request_data_1, sizeof(request_data_1));

    http::request http_request;
    boost::system::error_code ec;
    BOOST_CHECK(request_parser.parse(http_request, ec));
    BOOST_CHECK(!ec);

    BOOST_CHECK_EQUAL(http_request.get_content_length(), 0UL);
    BOOST_CHECK_EQUAL(request_parser.get_total_bytes_read(), sizeof(request_data_1));
    BOOST_CHECK_EQUAL(request_parser.get_content_bytes_read(), 0UL);
}

BOOST_AUTO_TEST_CASE(testHTTPParserSimpleResponse)
{
    http::parser response_parser(false);
    response_parser.set_read_buffer((const char*)response_data_1, sizeof(response_data_1));

    http::response http_response;
    boost::system::error_code ec;
    BOOST_CHECK(response_parser.parse(http_response, ec));
    BOOST_CHECK(!ec);

    BOOST_CHECK_EQUAL(http_response.get_content_length(), 117UL);
    BOOST_CHECK_EQUAL(response_parser.get_total_bytes_read(), sizeof(response_data_1));
    BOOST_CHECK_EQUAL(response_parser.get_content_bytes_read(), 117UL);

    boost::regex content_regex("^GIF89a.*");
    BOOST_CHECK(boost::regex_match(http_response.get_content(), content_regex));
}

BOOST_AUTO_TEST_CASE(testHTTPParserBadRequest)
{
    http::parser request_parser(true);
    request_parser.set_read_buffer((const char*)request_data_bad, sizeof(request_data_bad));

    http::request http_request;
    boost::system::error_code ec;
    BOOST_CHECK(!request_parser.parse(http_request, ec));
    BOOST_CHECK_EQUAL(ec.value(), http::parser::ERROR_VERSION_CHAR);
    BOOST_CHECK_EQUAL(ec.message(), "invalid version character");
}

BOOST_AUTO_TEST_CASE(testHTTPParserSimpleResponseWithSmallerMaxSize)
{
    http::parser response_parser(false);
    response_parser.set_read_buffer((const char*)response_data_1, sizeof(response_data_1));
    response_parser.set_max_content_length(4);

    http::response http_response;
    boost::system::error_code ec;
    BOOST_CHECK(response_parser.parse(http_response, ec));
    BOOST_CHECK(!ec);

    BOOST_CHECK_EQUAL(http_response.get_content_length(), 4UL);
    BOOST_CHECK_EQUAL(response_parser.get_total_bytes_read(), sizeof(response_data_1));
    BOOST_CHECK_EQUAL(response_parser.get_content_bytes_read(), 117UL);

    std::string content_str("GIF8");
    BOOST_CHECK_EQUAL(content_str, http_response.get_content());
}

BOOST_AUTO_TEST_CASE(testHTTPParserSimpleResponseWithZeroMaxSize)
{
    http::parser response_parser(false);
    response_parser.set_read_buffer((const char*)response_data_1, sizeof(response_data_1));
    response_parser.set_max_content_length(0);

    http::response http_response;
    boost::system::error_code ec;
    BOOST_CHECK(response_parser.parse(http_response, ec));
    BOOST_CHECK(!ec);

    BOOST_CHECK_EQUAL(http_response.get_content_length(), 0UL);
    BOOST_CHECK_EQUAL(response_parser.get_total_bytes_read(), sizeof(response_data_1));
    BOOST_CHECK_EQUAL(response_parser.get_content_bytes_read(), 117UL);

    BOOST_CHECK_EQUAL(http_response.get_content()[0], '\0');
}

BOOST_AUTO_TEST_CASE(testHTTPParser_MultipleResponseFrames)
{
    const unsigned char* frames[] = { resp2_frame0, resp2_frame1, resp2_frame2, 
            resp2_frame3, resp2_frame4, resp2_frame5, resp2_frame6 };

    size_t sizes[] = { sizeof(resp2_frame0), sizeof(resp2_frame1), sizeof(resp2_frame2), 
            sizeof(resp2_frame3), sizeof(resp2_frame4), sizeof(resp2_frame5), sizeof(resp2_frame6) };

    int frame_cnt = sizeof(frames)/sizeof(frames[0]);

    http::parser response_parser(false);
    http::response http_response;
    boost::system::error_code ec;

    boost::uint64_t total_bytes = 0;
    for (int i=0; i <  frame_cnt - 1; i++ ) {
        response_parser.set_read_buffer((const char*)frames[i], sizes[i]);
        BOOST_CHECK( boost::indeterminate(response_parser.parse(http_response, ec)) );
        BOOST_CHECK(!ec);
        total_bytes += sizes[i];
    }

    response_parser.set_read_buffer((const char*)frames[frame_cnt - 1], sizes[frame_cnt - 1]);
    BOOST_CHECK( response_parser.parse(http_response, ec) );
        BOOST_CHECK(!ec);
    total_bytes += sizes[frame_cnt - 1];

    BOOST_CHECK_EQUAL(http_response.get_content_length(), 4712UL);
    BOOST_CHECK_EQUAL(response_parser.get_total_bytes_read(), total_bytes);
    BOOST_CHECK_EQUAL(response_parser.get_content_bytes_read(), 4712UL);

    boost::regex content_regex(".*<title>Atomic\\sLabs:.*");
    BOOST_CHECK(boost::regex_match(http_response.get_content(), content_regex));
}

<<<<<<< HEAD
=======
BOOST_AUTO_TEST_CASE(testHTTPParserWithSemicolons)
{
    http::parser request_parser(true);
    request_parser.set_read_buffer((const char*)chunked_request_with_semicolon,
                                   sizeof(chunked_request_with_semicolon));
    
    http::request http_request;
    boost::system::error_code ec;
    BOOST_CHECK(request_parser.parse(http_request, ec));
    BOOST_CHECK(!ec);
    
    // The content length should be 15 and the ignored data after ';'
    // should not be added to content length
    BOOST_CHECK_EQUAL(http_request.get_content_length(), 15UL);
    BOOST_CHECK_EQUAL(request_parser.get_total_bytes_read(), sizeof(chunked_request_with_semicolon));
    BOOST_CHECK_EQUAL(request_parser.get_content_bytes_read(), 48UL);
    
}

BOOST_AUTO_TEST_CASE(testHTTPParserWithFooters)
{
    http::parser request_parser(true);
    request_parser.set_read_buffer((const char*)chunked_request_with_footers,
                                   sizeof(chunked_request_with_footers));
    
    http::request http_request;
    boost::system::error_code ec;
    BOOST_CHECK(request_parser.parse(http_request, ec));
    BOOST_CHECK(!ec);
    
    BOOST_CHECK_EQUAL(http_request.get_content_length(), 15UL);
    BOOST_CHECK_EQUAL(request_parser.get_total_bytes_read(), sizeof(chunked_request_with_footers));
    BOOST_CHECK_EQUAL(request_parser.get_content_bytes_read(), 28UL);
    BOOST_CHECK_EQUAL(http_request.get_header("Transfer-Encoding"), "chunked");
    
    // Check if the footers are added as a part of the HTTP Data
    BOOST_CHECK_EQUAL(http_request.get_header("some-footer"), "some-value");
    BOOST_CHECK_EQUAL(http_request.get_header("another-footer"), "another-value");
}

BOOST_AUTO_TEST_CASE(testHTTPParserWithErrorInFooters)
{
    http::parser request_parser(true);
    request_parser.set_read_buffer((const char*)chunked_request_with_error_in_footers,
                                   sizeof(chunked_request_with_error_in_footers));
    
    http::request http_request;
    boost::system::error_code ec;
    
    // The HTTP Packet does not contain any footer value associated with the footer key
    // This will lead to any error within the parse_headers() method
    BOOST_CHECK_EQUAL(request_parser.parse(http_request, ec), false);
    
    // Check if there is an error generated
    BOOST_CHECK_EQUAL(ec.value(), http::parser::ERROR_HEADER_CHAR);

    BOOST_CHECK_EQUAL(http_request.get_content_length(), 15UL);
    BOOST_CHECK_EQUAL(request_parser.get_total_bytes_read(), 84UL);
    BOOST_CHECK_EQUAL(http_request.get_header("Transfer-Encoding"), "chunked");
    
    // Check if the footers are added as a part of the HTTP Data
    BOOST_CHECK_EQUAL(http_request.get_header("some-footer"), "some-value");
}

>>>>>>> upstream/develop

/// fixture used for testing http::parser's X-Fowarded-For header parsing
class HTTPParserForwardedForTests_F
{
public:
    HTTPParserForwardedForTests_F(void) {}
    ~HTTPParserForwardedForTests_F(void) {}

    inline void checkParsingTrue(const std::string& header, const std::string& result) {
        std::string public_ip;
        BOOST_CHECK(http::parser::parse_forwarded_for(header, public_ip));
        BOOST_CHECK_EQUAL(public_ip, result);
    }

    inline void checkParsingFalse(const std::string& header) {
        std::string public_ip;
        BOOST_CHECK(! http::parser::parse_forwarded_for(header, public_ip));
    }
};

BOOST_FIXTURE_TEST_SUITE(HTTPParserForwardedForTests_S, HTTPParserForwardedForTests_F)

BOOST_AUTO_TEST_CASE(checkParseForwardedForHeaderNoIP) {
    checkParsingFalse("myserver");
    checkParsingFalse("128.2.02f.12");
}

BOOST_AUTO_TEST_CASE(checkParseForwardedForHeaderNotPublic) {
    checkParsingFalse("127.0.0.1");
    checkParsingFalse("10.0.2.1");
    checkParsingFalse("192.168.2.12");
    checkParsingFalse("172.16.2.1");
    checkParsingFalse("172.21.2.1");
    checkParsingFalse("172.30.2.1");
}

BOOST_AUTO_TEST_CASE(checkParseForwardedForHeaderWithSpaces) {
    checkParsingTrue("   129.12.12.204   ", "129.12.12.204");
}

BOOST_AUTO_TEST_CASE(checkParseForwardedForHeaderFirstNotIP) {
    checkParsingTrue(" phono , 129.2.31.24, 62.31.21.2", "129.2.31.24");
    checkParsingTrue("not_ipv4, 127.2.31.24, 62.31.21.2", "62.31.21.2");
}

BOOST_AUTO_TEST_CASE(checkParseForwardedForHeaderFirstNotPublic) {
    checkParsingTrue("127.0.0.1, 62.31.21.2", "62.31.21.2");
    checkParsingTrue("10.21.31.2, 172.15.31.2", "172.15.31.2");
    checkParsingTrue("192.168.2.12, 172.32.31.2", "172.32.31.2");
}

BOOST_AUTO_TEST_SUITE_END()
