// ------------------------------------------------------------------
// pion-net: a C++ framework for building lightweight HTTP interfaces
// ------------------------------------------------------------------
// Copyright (C) 2007-2008 Atomic Labs, Inc.  (http://www.atomiclabs.com)
//
// Distributed under the Boost Software License, Version 1.0.
// See http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <pion/net/HTTPTypes.hpp>
#include <ctime>


namespace pion {		// begin namespace pion
namespace net {		// begin namespace net (Pion Network Library)


// generic strings used by HTTP
const std::string	HTTPTypes::STRING_EMPTY;
const std::string	HTTPTypes::STRING_CRLF("\x0D\x0A");
const std::string	HTTPTypes::STRING_HTTP_VERSION("HTTP/1.1");
const std::string	HTTPTypes::HEADER_NAME_VALUE_DELIMITER(": ");

// common HTTP header names
const std::string	HTTPTypes::HEADER_HOST("Host");
const std::string	HTTPTypes::HEADER_COOKIE("Cookie");
const std::string	HTTPTypes::HEADER_SET_COOKIE("Set-Cookie");
const std::string	HTTPTypes::HEADER_CONNECTION("Connection");
const std::string	HTTPTypes::HEADER_CONTENT_TYPE("Content-Type");
const std::string	HTTPTypes::HEADER_CONTENT_LENGTH("Content-Length");
const std::string	HTTPTypes::HEADER_CONTENT_LOCATION("Content-Location");
const std::string	HTTPTypes::HEADER_LAST_MODIFIED("Last-Modified");
const std::string	HTTPTypes::HEADER_IF_MODIFIED_SINCE("If-Modified-Since");
const std::string	HTTPTypes::HEADER_TRANSFER_ENCODING("Transfer-Encoding");
const std::string	HTTPTypes::HEADER_LOCATION("Location");

// common HTTP content types
const std::string	HTTPTypes::CONTENT_TYPE_HTML("text/html");
const std::string	HTTPTypes::CONTENT_TYPE_TEXT("text/plain");
const std::string	HTTPTypes::CONTENT_TYPE_XML("text/xml");
const std::string	HTTPTypes::CONTENT_TYPE_URLENCODED("application/x-www-form-urlencoded");

// common HTTP request methods
const std::string	HTTPTypes::REQUEST_METHOD_HEAD("HEAD");
const std::string	HTTPTypes::REQUEST_METHOD_GET("GET");
const std::string	HTTPTypes::REQUEST_METHOD_PUT("PUT");
const std::string	HTTPTypes::REQUEST_METHOD_POST("POST");
const std::string	HTTPTypes::REQUEST_METHOD_DELETE("DELETE");

// common HTTP response messages
const std::string	HTTPTypes::RESPONSE_MESSAGE_OK("OK");
const std::string	HTTPTypes::RESPONSE_MESSAGE_CREATED("Created");
const std::string	HTTPTypes::RESPONSE_MESSAGE_NO_CONTENT("No Content");
const std::string	HTTPTypes::RESPONSE_MESSAGE_FORBIDDEN("Forbidden");
const std::string	HTTPTypes::RESPONSE_MESSAGE_NOT_FOUND("Not Found");
const std::string	HTTPTypes::RESPONSE_MESSAGE_METHOD_NOT_ALLOWED("Method Not Allowed");
const std::string	HTTPTypes::RESPONSE_MESSAGE_NOT_MODIFIED("Not Modified");
const std::string	HTTPTypes::RESPONSE_MESSAGE_BAD_REQUEST("Bad Request");
const std::string	HTTPTypes::RESPONSE_MESSAGE_SERVER_ERROR("Server Error");
const std::string	HTTPTypes::RESPONSE_MESSAGE_NOT_IMPLEMENTED("Not Implemented");

// common HTTP response codes
const unsigned int	HTTPTypes::RESPONSE_CODE_OK = 200;
const unsigned int	HTTPTypes::RESPONSE_CODE_CREATED = 201;
const unsigned int	HTTPTypes::RESPONSE_CODE_NO_CONTENT = 204;
const unsigned int	HTTPTypes::RESPONSE_CODE_FORBIDDEN = 403;
const unsigned int	HTTPTypes::RESPONSE_CODE_NOT_FOUND = 404;
const unsigned int	HTTPTypes::RESPONSE_CODE_METHOD_NOT_ALLOWED = 405;
const unsigned int	HTTPTypes::RESPONSE_CODE_NOT_MODIFIED = 304;
const unsigned int	HTTPTypes::RESPONSE_CODE_BAD_REQUEST = 400;
const unsigned int	HTTPTypes::RESPONSE_CODE_SERVER_ERROR = 500;
const unsigned int	HTTPTypes::RESPONSE_CODE_NOT_IMPLEMENTED = 501;


// static member functions
	
std::string HTTPTypes::url_decode(const std::string& str)
{
	char decode_buf[3];
	std::string result;
	result.reserve(str.size());
	
	for (std::string::size_type pos = 0; pos < str.size(); ++pos) {
		switch(str[pos]) {
		case '+':
			// convert to space character
			result += ' ';
			break;
		case '%':
			// decode hexidecimal value
			if (pos + 2 < str.size()) {
				decode_buf[0] = str[++pos];
				decode_buf[1] = str[++pos];
				decode_buf[2] = '\0';
				result += static_cast<char>( strtol(decode_buf, 0, 16) );
			} else {
				// recover from error by not decoding character
				result += '%';
			}
			break;
		default:
			// character does not need to be escaped
			result += str[pos];
		}
	};
	
	return result;
}
	
std::string HTTPTypes::url_encode(const std::string& str)
{
	char encode_buf[4];
	std::string result;
	encode_buf[0] = '%';
	result.reserve(str.size());

	// character selection for this algorithm is based on the following url:
	// http://www.blooberry.com/indexdot/html/topics/urlencoding.htm
	
	for (std::string::size_type pos = 0; pos < str.size(); ++pos) {
		switch(str[pos]) {
		default:
			if (str[pos] >= 32 && str[pos] < 127) {
				// character does not need to be escaped
				result += str[pos];
				break;
			}
			// else pass through to next case
			
		case '$': case '&': case '+': case ',': case '/': case ':':
		case ';': case '=': case '?': case '@': case '"': case '<':
		case '>': case '#': case '%': case '{': case '}': case '|':
		case '\\': case '^': case '~': case '[': case ']': case '`':
			// the character needs to be encoded
			sprintf(encode_buf+1, "%2X", str[pos]);
			result += encode_buf;
			break;
		}
	};
	
	return result;
}	

std::string HTTPTypes::get_date_string(const time_t t)
{
	// use mutex since time functions are normally not thread-safe
	static boost::mutex	time_mutex;
	static const char *TIME_FORMAT = "%a, %d %b %Y %H:%M:%S GMT";
	static const unsigned int TIME_BUF_SIZE = 100;
	char time_buf[TIME_BUF_SIZE+1];

	boost::mutex::scoped_lock time_lock(time_mutex);
	if (strftime(time_buf, TIME_BUF_SIZE, TIME_FORMAT, gmtime(&t)) == 0)
		time_buf[0] = '\0';	// failed; resulting buffer is indeterminate
	time_lock.unlock();

	return std::string(time_buf);
}

std::string HTTPTypes::make_query_string(const QueryParams& query_params)
{
	std::string query_string;
	for (QueryParams::const_iterator i = query_params.begin(); i != query_params.end(); ++i) {
		if (i != query_params.begin())
			query_string += '&';
		query_string += url_encode(i->first);
		query_string += '=';
		query_string += url_encode(i->second);
	}
	return query_string;
}

std::string HTTPTypes::make_set_cookie_header(const std::string& name,
											  const std::string& value,
											  const std::string& path,
											  const bool has_max_age,
											  const unsigned long max_age)
{
	std::string set_cookie_header(name);
	set_cookie_header += "=\"";
	set_cookie_header += value;
	set_cookie_header += "\"; Version=\"1\"";
	if (! path.empty()) {
		set_cookie_header += "; Path=\"";
		set_cookie_header += path;
		set_cookie_header += '\"';
	}
	if (has_max_age) {
		set_cookie_header += "; Max-Age=\"";
		set_cookie_header += boost::lexical_cast<std::string>(max_age);
		set_cookie_header += '\"';
	}
	return set_cookie_header;
}

	
}	// end namespace net
}	// end namespace pion

