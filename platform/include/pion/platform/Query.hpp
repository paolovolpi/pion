// ------------------------------------------------------------------------
// Pion is a development platform for building Reactors that process Events
// ------------------------------------------------------------------------
// Copyright (C) 2007-2008 Atomic Labs, Inc.  (http://www.atomiclabs.com)
//
// Pion is free software: you can redistribute it and/or modify it under the
// terms of the GNU Affero General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// Pion is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
// more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with Pion.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef __PION_QUERY_HEADER__
#define __PION_QUERY_HEADER__

#include <cstdio>
#include <string>
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <pion/PionConfig.hpp>
#include <pion/platform/Vocabulary.hpp>
#include <pion/platform/Event.hpp>


namespace pion {		// begin namespace pion
namespace platform {	// begin namespace platform (Pion Platform Library)

// forward declarations used by Query
class Database;


///
/// Query: abstract class for representing a compiled database query
///
class Query
	: private boost::noncopyable
{
public:

	/// data type for a pair where first is a field name, and second is Term info
//	typedef std::pair<std::string, Vocabulary::Term>		FieldData;
	typedef std::pair<std::string, Vocabulary::Term>		FieldData;

	/// data type for a map of Term references to database field names
//	typedef std::map<Vocabulary::TermRef, FieldData>		FieldMap;
	typedef std::vector<FieldData>							FieldMap;


	/// virtual destructor -> this class is just an interface
	virtual ~Query() {}

	/**
	 * binds a NULL value to a query parameter
	 *
	 * @param param the query parameter number to which the value will be bound (starting with 0)
	 */
	virtual void bindNull(unsigned int param) = 0;

	/**
	 * binds a std::string value to a query parameter
	 *
	 * @param param the query parameter number to which the value will be bound (starting with 0)
	 * @param value the value to bind to the query parameter
	 * @param copy_value if true, the string will be copied into a temporary buffer
	 */
	virtual void bindString(unsigned int param, const std::string& value, bool copy_value = true) = 0;

	virtual void fetchString(unsigned int param, std::string& value) = 0;

	/**
	 * binds a string (const char *) value to a query parameter
	 *
	 * @param param the query parameter number to which the value will be bound (starting with 0)
	 * @param value the value to bind to the query parameter
	 * @param copy_value if true, the string will be copied into a temporary buffer
	 */
	virtual void bindString(unsigned int param, const char *value, bool copy_value = true) = 0;

//	virtual char *fetchString(unsigned int param) = 0;

	/**
	 * binds an integer value to a query parameter
	 *
	 * @param param the query parameter number to which the value will be bound (starting with 0)
	 * @param value the value to bind to the query parameter
	 */
	virtual void bindInt(unsigned int param, const boost::int32_t value) = 0;

	virtual boost::int32_t fetchInt(unsigned int param) = 0;

	/**
	 * binds an unsigned integer value to a query parameter
	 *
	 * @param param the query parameter number to which the value will be bound (starting with 0)
	 * @param value the value to bind to the query parameter
	 */
	virtual void bindUInt(unsigned int param, const boost::uint32_t value) = 0;

	virtual boost::uint32_t fetchUInt(unsigned int param) = 0;

	/**
	 * binds a big integer value to a query parameter
	 *
	 * @param param the query parameter number to which the value will be bound (starting with 0)
	 * @param value the value to bind to the query parameter
	 */
	virtual void bindBigInt(unsigned int param, const boost::int64_t value) = 0;

	virtual boost::int64_t fetchBigInt(unsigned int param) = 0;

	/**
	 * binds an unsigned big integer value to a query parameter
	 *
	 * @param param the query parameter number to which the value will be bound (starting with 0)
	 * @param value the value to bind to the query parameter
	 */
	virtual void bindUBigInt(unsigned int param, const boost::uint64_t value) = 0;

	virtual boost::uint64_t fetchUBigInt(unsigned int param) = 0;

	/**
	 * binds a floating point number value to a query parameter
	 *
	 * @param param the query parameter number to which the value will be bound (starting with 0)
	 * @param value the value to bind to the query parameter
	 */
	virtual void bindFloat(unsigned int param, const float value) = 0;

	virtual float fetchFloat(unsigned int param) = 0;

	/**
	 * binds a double floating point number value to a query parameter
	 *
	 * @param param the query parameter number to which the value will be bound (starting with 0)
	 * @param value the value to bind to the query parameter
	 */
	virtual void bindDouble(unsigned int param, const double value) = 0;

	virtual double fetchDouble(unsigned int param) = 0;

	/**
	 * binds a long double floating point number value to a query parameter
	 *
	 * @param param the query parameter number to which the value will be bound (starting with 0)
	 * @param value the value to bind to the query parameter
	 */
	virtual void bindLongDouble(unsigned int param, const long double value) = 0;

	virtual long double fetchLongDouble(unsigned int param) = 0;

	/**
	 * binds a date_time value to a query parameter
	 *
	 * @param param the query parameter number to which the value will be bound (starting with 0)
	 * @param value the value to bind to the query parameter
	 */
	virtual void bindDateTime(unsigned int param, const PionDateTime& value) = 0;

	virtual void fetchDateTime(unsigned int param, PionDateTime& val) = 0;

	/**
	 * binds a date value to a query parameter
	 *
	 * @param param the query parameter number to which the value will be bound (starting with 0)
	 * @param value the value to bind to the query parameter
	 */
	virtual void bindDate(unsigned int param, const PionDateTime& value) = 0;

	virtual void fetchDate(unsigned int param, PionDateTime& val) = 0;

	/**
	 * binds a time value to a query parameter
	 *
	 * @param param the query parameter number to which the value will be bound (starting with 0)
	 * @param value the value to bind to the query parameter
	 */
	virtual void bindTime(unsigned int param, const PionDateTime& value) = 0;

	virtual void fetchTime(unsigned int param, PionDateTime& val) = 0;

	/**
	 * binds the data contained within an Event to the query parameters
	 * (this asssumes that field_map is ordered the same as the parameters)
	 *
	 * @param field_map mapping of Vocabulary Terms to Database fields
	 * @param e the Event containing data to bind to the query
	 * @param copy_strings if true, the strings will be copied into temporary buffers
	 */
	inline void bindEvent(const FieldMap& field_map, const Event& e, bool copy_strings = true);

	inline void fetchEvent(const FieldMap& field_map, EventPtr e);

	/**
	 * runs the compiled query
	 *
	 * @return true if there is a result row available
	 */
	virtual bool run(void) = 0;

	/**
	 * runs a generic query
 	*
 	* @param ins a FieldMap of input fields
 	* @param src input event, used for ins FieldMap
 	* @param outs a FieldMap of output fields
 	* @param dest output event, used for outs FieldMap
	* @param limit how many results max
 	*
 	* @return bool if output event was modified
 	*/
	virtual bool runFullQuery(const FieldMap& ins, const EventPtr& src,
			const FieldMap& outs, EventPtr& dest,unsigned int limit) = 0;

	/**
	 * gets more results for generic query
 	*
 	* @param outs a FieldMap of output fields
 	* @param dest output event, used for outs FieldMap
	* @param limit how many results max
 	*
 	* @return bool if output event was modified
 	*/
	virtual bool runFullGetMore(const FieldMap& outs, EventPtr& dest,unsigned int limit) = 0;

	/// resets the compiled query so that it can be run again
	virtual void reset(void) = 0;

	/// returns the SQL that was compiled to initialize this query
	inline const std::string& getSQL(void) const { return m_sql_query; }


protected:

	/**
	 * this class can only be constructed by its descendants
	 *
	 * @param sql_query the SQL to use to initialize this query
	 */
	Query(const std::string& sql_query)
		: m_sql_query(sql_query)
	{}

	/**
	 * writes simple date string into buffer (%Y-%m-%d)
	 *
	 * @param buf buffer to write to (size must be > 10 bytes)
	 * @param t timestamp to use for writing
	 */
	static void writeDateString(char *buf, const pion::PionDateTime& t) {
		sprintf(buf, "%d-%d-%d",
			static_cast<int>( t.date().year() ),
			static_cast<int>( t.date().month() ),
			static_cast<int>( t.date().day() ));
	}

	/**
	 * writes simple time string into buffer (%H-%M-%S)
	 *
	 * @param buf buffer to write to (size must be > 8 bytes)
	 * @param t timestamp to use for writing
	 */
	static void writeTimeString(char *buf, const pion::PionDateTime& t) {
		sprintf(buf, "%d:%d:%d",
			static_cast<int>( t.time_of_day().hours() ),
			static_cast<int>( t.time_of_day().minutes() ),
			static_cast<int>( t.time_of_day().seconds() ));
	}

	/**
	 * writes simple date & time string into buffer (%Y-%m-%d %H-%M-%S)
	 *
	 * @param buf buffer to write to (size must be > 19 bytes)
	 * @param t timestamp to use for writing
	 */
	static void writeDateTimeString(char *buf, const pion::PionDateTime& t) {
		sprintf(buf, "%d-%d-%d %d:%d:%d",
			static_cast<int>( t.date().year() ),
			static_cast<int>( t.date().month() ),
			static_cast<int>( t.date().day() ),
			static_cast<int>( t.time_of_day().hours() ),
			static_cast<int>( t.time_of_day().minutes() ),
			static_cast<int>( t.time_of_day().seconds() ));
	}
	
	/**
	 * return static buffer with date string (%Y-%m-%d)
	 *
	 * @param t timestamp to use for writing
	 *
	 * @return char * static buffer containing output (not thread safe, must be used immediately)
	 */
	inline char *getDateString(const pion::PionDateTime& t) const {
		writeDateString(m_time_buf, t);
		return m_time_buf;
	}

	/**
	 * return static buffer with time string (%H-%M-%S)
	 *
	 * @param buf buffer to write to (size must be > 8 bytes)
	 *
	 * @return char * static buffer containing output (not thread safe, must be used immediately)
	 */
	inline char *getTimeString(const pion::PionDateTime& t) const {
		writeTimeString(m_time_buf, t);
		return m_time_buf;
	}

	/**
	 * return static buffer with date and time string (%Y-%m-%d %H-%M-%S)
	 *
	 * @param buf buffer to write to (size must be > 19 bytes)
	 *
	 * @return char * static buffer containing output (not thread safe, must be used immediately)
	 */
	inline char *getDateTimeString(const pion::PionDateTime& t) const {
		writeDateTimeString(m_time_buf, t);
		return m_time_buf;
	}


private:

	/// the SQL that was compiled to initialize this query
	const std::string					m_sql_query;

	/// buffer used to serialize PionDateTime objects for databases
	mutable char						m_time_buf[25];
};


// inline member functions for Query

inline void Query::bindEvent(const FieldMap& field_map, const Event& e, bool copy_strings)
{
	for (unsigned int param = 0; param < field_map.size(); param++) {
		const Event::ParameterValue *value_ptr = e.getPointer(field_map[param].second.term_ref);
		if (value_ptr == NULL) {
			bindNull(param);
		} else {
			switch(field_map[param].second.term_type) {
				case Vocabulary::TYPE_NULL:
				case Vocabulary::TYPE_OBJECT:
					bindNull(param);
					break;
				case Vocabulary::TYPE_INT8:
				case Vocabulary::TYPE_INT16:
				case Vocabulary::TYPE_INT32:
					bindInt(param, boost::get<boost::int32_t>(*value_ptr));
					break;
				case Vocabulary::TYPE_INT64:
					bindBigInt(param, boost::get<boost::int64_t>(*value_ptr));
					break;
				case Vocabulary::TYPE_UINT8:
				case Vocabulary::TYPE_UINT16:
				case Vocabulary::TYPE_UINT32:
					bindUInt(param, boost::get<boost::uint32_t>(*value_ptr));
					break;
				case Vocabulary::TYPE_UINT64:
					bindUBigInt(param, boost::get<boost::uint64_t>(*value_ptr));
					break;
				case Vocabulary::TYPE_FLOAT:
					bindFloat(param, boost::get<float>(*value_ptr));
					break;
				case Vocabulary::TYPE_DOUBLE:
					bindDouble(param, boost::get<double>(*value_ptr));
					break;
				case Vocabulary::TYPE_LONG_DOUBLE:
					bindLongDouble(param, boost::get<long double>(*value_ptr));
					break;
				case Vocabulary::TYPE_SHORT_STRING:
				case Vocabulary::TYPE_STRING:
				case Vocabulary::TYPE_LONG_STRING:
				case Vocabulary::TYPE_CHAR:
				case Vocabulary::TYPE_REGEX:
					bindString(param,
						boost::get<const Event::SimpleString&>(*value_ptr).get(),
						copy_strings);
					break;
				case Vocabulary::TYPE_DATE_TIME:
					bindDateTime(param, boost::get<const PionDateTime&>(*value_ptr));
					break;
				case Vocabulary::TYPE_DATE:
					bindDate(param, boost::get<const PionDateTime&>(*value_ptr));
					break;
				case Vocabulary::TYPE_TIME:
					bindTime(param, boost::get<const PionDateTime&>(*value_ptr));
					break;
			}
		}
	}
}

inline void Query::fetchEvent(const FieldMap& field_map, EventPtr e)
{
	for (unsigned int param = 0; param < field_map.size(); param++) {
		switch(field_map[param].second.term_type) {
			case Vocabulary::TYPE_NULL:
			case Vocabulary::TYPE_OBJECT:
				break;
			case Vocabulary::TYPE_INT8:
			case Vocabulary::TYPE_INT16:
			case Vocabulary::TYPE_INT32:
				e->setInt(field_map[param].second.term_ref, fetchInt(param));
				break;
			case Vocabulary::TYPE_INT64:
				e->setBigInt(field_map[param].second.term_ref, fetchBigInt(param));
				break;
			case Vocabulary::TYPE_UINT8:
			case Vocabulary::TYPE_UINT16:
			case Vocabulary::TYPE_UINT32:
				e->setUInt(field_map[param].second.term_ref, fetchUInt(param));
				break;
			case Vocabulary::TYPE_UINT64:
				e->setUBigInt(field_map[param].second.term_ref, fetchUBigInt(param));
				break;
			case Vocabulary::TYPE_FLOAT:
				e->setFloat(field_map[param].second.term_ref, fetchFloat(param));
				break;
			case Vocabulary::TYPE_DOUBLE:
				e->setDouble(field_map[param].second.term_ref, fetchDouble(param));
				break;
			case Vocabulary::TYPE_LONG_DOUBLE:
				e->setLongDouble(field_map[param].second.term_ref, fetchLongDouble(param));
				break;
			case Vocabulary::TYPE_SHORT_STRING:
			case Vocabulary::TYPE_STRING:
			case Vocabulary::TYPE_LONG_STRING:
			case Vocabulary::TYPE_CHAR:
			case Vocabulary::TYPE_REGEX:
				{
					std::string val;
					fetchString(param, val);
					e->setString(field_map[param].second.term_ref, val);
				}
				break;
			case Vocabulary::TYPE_DATE_TIME:
				{
					PionDateTime val;
					fetchDateTime(param, val);
					e->setDateTime(field_map[param].second.term_ref, val);
				}
				break;
			case Vocabulary::TYPE_DATE:
				{
					PionDateTime val;
					fetchDate(param, val);
					e->setDateTime(field_map[param].second.term_ref, val);
				}
				break;
			case Vocabulary::TYPE_TIME:
				{
					PionDateTime val;
					fetchTime(param, val);
					e->setDateTime(field_map[param].second.term_ref, val);
				}
				break;
		}
	}
}


/// data type used to uniquely identify a specific type of database query
typedef std::string						QueryID;


/// data type used for Query smart pointers
typedef boost::shared_ptr<Query>		QueryPtr;


}	// end namespace platform
}	// end namespace pion

#endif
