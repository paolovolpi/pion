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
// WARRANTY; withouDatabaseOutputt even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
// more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with Pion.  If not, see <http://www.gnu.org/licenses/>.
//

#include <pion/platform/Reactor.hpp>
#include <pion/platform/ConfigManager.hpp>
#include <pion/platform/DatabaseManager.hpp>
#include "DatabaseOutputReactor.hpp"

using namespace pion::platform;


namespace pion {		// begin namespace pion
namespace plugins {		// begin namespace plugins


// static members of DatabaseOutputReactor

const std::string			DatabaseOutputReactor::DATABASE_ELEMENT_NAME = "Database";
const std::string			DatabaseOutputReactor::TABLE_ELEMENT_NAME = "Table";
const std::string			DatabaseOutputReactor::FIELD_ELEMENT_NAME = "Field";
const std::string			DatabaseOutputReactor::TERM_ATTRIBUTE_NAME = "term";

	
// DatabaseOutputReactor member functions

void DatabaseOutputReactor::setConfig(const Vocabulary& v, const xmlNodePtr config_ptr)
{
	// first set config options for the Reactor base class
	boost::mutex::scoped_lock reactor_lock(m_mutex);
	Reactor::setConfig(v, config_ptr);
	
	// get the database to use
	if (! ConfigManager::getConfigOption(DATABASE_ELEMENT_NAME, m_database_id, config_ptr))
		throw EmptyDatabaseException(getId());
	m_database_ptr = getDatabaseManager().getDatabase(m_database_id);
	PION_ASSERT(m_database_ptr);
	
	// get the name of the table to store events in
	if (! ConfigManager::getConfigOption(TABLE_ELEMENT_NAME, m_table_name, config_ptr))
		throw EmptyTableException(getId());
	
	// next, map the database fields to Terms
	m_field_map.clear();
	xmlNodePtr field_node = config_ptr;
	while ( (field_node = ConfigManager::findConfigNodeByName(FIELD_ELEMENT_NAME, field_node)) != NULL)
	{
		// parse new field mapping
		
		// start with the name of the field (element content)
		xmlChar *xml_char_ptr = xmlNodeGetContent(field_node);
		if (xml_char_ptr == NULL || xml_char_ptr[0]=='\0') {
			if (xml_char_ptr != NULL)
				xmlFree(xml_char_ptr);
			throw EmptyFieldException(getId());
		}
		const std::string field_name(reinterpret_cast<char*>(xml_char_ptr));
		xmlFree(xml_char_ptr);
		
		// next get the Term we want to map to
		xml_char_ptr = xmlGetProp(field_node, reinterpret_cast<const xmlChar*>(TERM_ATTRIBUTE_NAME.c_str()));
		if (xml_char_ptr == NULL || xml_char_ptr[0]=='\0') {
			if (xml_char_ptr != NULL)
				xmlFree(xml_char_ptr);
			throw EmptyTermException(getId());
		}
		const std::string term_id(reinterpret_cast<char*>(xml_char_ptr));
		xmlFree(xml_char_ptr);
		
		// make sure that the Term is valid
		const Vocabulary::TermRef term_ref = v.findTerm(term_id);
		if (term_ref == Vocabulary::UNDEFINED_TERM_REF)
			throw UnknownTermException(term_id);
		
		// add the field mapping
		m_field_map[term_ref] = field_name;
		
		// step to the next field mapping
		field_node = field_node->next;
	}
	
	// create the database table if it does not yet exist
	createTable();
}
	
void DatabaseOutputReactor::updateVocabulary(const Vocabulary& v)
{
	// first update anything in the Reactor base class that might be needed
	boost::mutex::scoped_lock reactor_lock(m_mutex);
	Reactor::updateVocabulary(v);
}
	
void DatabaseOutputReactor::operator()(const EventPtr& e)
{
	boost::mutex::scoped_lock reactor_lock(m_mutex);
	incrementEventsIn();

	PION_ASSERT(m_database_ptr);
	
	// add a record for the event in the database output table
	
	// ...
	
	
	deliverEvent(e);
}
	
void DatabaseOutputReactor::createTable(void)
{
	// build a SQL query to create the output table if it doesn't yet exist
	std::string create_query = "CREATE TABLE IF NOT EXISTS ";
	create_query += m_table_name;
	create_query += " ( ";
	
	
	
	// ...
	
	
	
	create_query += " )";
	
	// ...
	
}
		
	
}	// end namespace plugins
}	// end namespace pion


/// creates new DatabaseOutputReactor objects
extern "C" PION_PLUGIN_API pion::platform::Reactor *pion_create_DatabaseOutputReactor(void) {
	return new pion::plugins::DatabaseOutputReactor();
}

/// destroys DatabaseOutputReactor objects
extern "C" PION_PLUGIN_API void pion_destroy_DatabaseOutputReactor(pion::plugins::DatabaseOutputReactor *reactor_ptr) {
	delete reactor_ptr;
}
