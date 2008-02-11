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

#include <pion/PionConfig.hpp>
#include <pion/platform/Event.hpp>
#include <pion/platform/Vocabulary.hpp>
#include <boost/test/unit_test.hpp>

using namespace pion;
using namespace pion::platform;


/// sets up logging (run once only)
extern void setup_logging_for_unit_tests(void);


class EventTests_F {
public:
	EventTests_F()
		: m_null_term("urn:pion:null-term"), m_plain_int_term("urn:pion:plain-old-int"),
		m_big_int_term("urn:pion:big-int"), m_fixed_term("urn:pion:fixed-text"),
		m_date_term("urn:pion:date"), m_object_term("urn:pion:simple-object")
	{
		setup_logging_for_unit_tests();
		// initialize our initial term set
		m_null_term.term_ref = static_cast<Vocabulary::TermRef>(1);
		m_plain_int_term.term_ref = static_cast<Vocabulary::TermRef>(2);
		m_big_int_term.term_ref = static_cast<Vocabulary::TermRef>(3);
		m_fixed_term.term_ref = static_cast<Vocabulary::TermRef>(4);
		m_object_term.term_ref = static_cast<Vocabulary::TermRef>(5);
		m_null_term.term_type = Vocabulary::TYPE_NULL;
		m_plain_int_term.term_type = Vocabulary::TYPE_INT16;
		m_big_int_term.term_type = Vocabulary::TYPE_UINT64;
		m_fixed_term.term_type = Vocabulary::TYPE_CHAR;
		m_date_term.term_type = Vocabulary::TYPE_DATE;
		m_object_term.term_type = Vocabulary::TYPE_OBJECT;
		m_null_term.term_comment = "A plain, old integer number";
		m_plain_int_term.term_comment = "A plain, old integer number";
		m_big_int_term.term_comment = "A really big positive integer";
		m_fixed_term.term_comment = "Ten bytes of text";
		m_date_term.term_comment = "A date";
		m_object_term.term_comment = "An object containing other Terms";
		m_fixed_term.term_size = 10;
		m_date_term.term_format = "%Y-%m-%d";
	}
	~EventTests_F() {
	}
	void addAllTerms() {
		m_vocabulary.addTerm(m_null_term);
		m_vocabulary.addTerm(m_plain_int_term);
		m_vocabulary.addTerm(m_big_int_term);
		m_vocabulary.addTerm(m_fixed_term);
		m_vocabulary.addTerm(m_date_term);
		m_vocabulary.addTerm(m_object_term);
	}

	Vocabulary			m_vocabulary;
	Vocabulary::Term	m_null_term;
	Vocabulary::Term	m_plain_int_term;
	Vocabulary::Term	m_big_int_term;
	Vocabulary::Term	m_fixed_term;
	Vocabulary::Term	m_date_term;
	Vocabulary::Term	m_object_term;
};

BOOST_FIXTURE_TEST_SUITE(EventTests_S, EventTests_F)

BOOST_AUTO_TEST_CASE(checkEventAssignmentValues) {
	addAllTerms();
	std::string short_msg_str("short msg");
	Event e(m_object_term.term_ref);
	e.setInt(m_plain_int_term.term_ref, 24);
	e.setUBigInt(m_big_int_term.term_ref, 2025221224);
	e[m_fixed_term.term_ref] = short_msg_str;
	e[m_date_term.term_ref] = PionDateTime(boost::gregorian::date(2007, 4, 5));

	const boost::any *value_ptr = e.getPointer(m_plain_int_term.term_ref);
	BOOST_REQUIRE(value_ptr != NULL);
	BOOST_CHECK_EQUAL(boost::any_cast<boost::int32_t>(*value_ptr), 24);
	BOOST_CHECK_EQUAL(boost::any_cast<boost::uint64_t>(e[m_big_int_term.term_ref]), 2025221224UL);
	BOOST_CHECK_EQUAL(e.getString(m_fixed_term.term_ref), short_msg_str);
	PionDateTime pdt = e.getDateTime(m_date_term.term_ref);
	BOOST_CHECK_EQUAL(pdt.date().year(), 2007);
	BOOST_CHECK_EQUAL(pdt.date().month(), 4);
	BOOST_CHECK_EQUAL(pdt.date().day(), 5);
}

BOOST_AUTO_TEST_SUITE_END()
