#!/usr/bin/python
# --------------------------------------
# pion configuration file upgrade script
# --------------------------------------

# Import libraries used by this script
import sys, os, re, shutil, optparse, uuid
from lxml import etree

# Base class used for configuration upgrade logic
class UpgradeRule(object):
	"""Class used to hold version upgrade logic"""
	def __init__(self, version, regex):
		self.version = version
		self.regex = regex
	def start(self, pion_config):
		"""this is always called before process"""
		if (not QUIET):
			print 'upgrading from ' + pion_config.version + ' to ' + self.version
	def process(self, pion_config):
		"""override this function to define upgrade logic"""
		pass
	def finish(self, pion_config):
		"""this is always called after process"""
		pion_config.set_version(self.version)


# Global list of configuration upgrade rules
RULES = list()


###########################################
# KEEP THIS UPDATED TO THE LATEST VERSION #
###########################################
CURRENT_VERSION = '3.1.2'

########################################
# CONFIGURATION UPGRADE RULES GO BELOW #
#VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV#

class Upgrade30xTo31x(UpgradeRule):
	"""Upgrade from 3.0.x to 3.1.x"""
	def __init__(self, version, regex):
		UpgradeRule.__init__(self, version, regex)
	def update_users(self, cfg):
		for u in cfg.root.iter('{%s}User' % PION_NS):
			if (VERBOSE): print 'Updating User configuration: ' + u.get('id')
			# iterate elements, get permissions and remove Permit nodes
			permissions = list()
			for n in u.getchildren():
				if (n.tag == ('{%s}Permit' % PION_NS)):
					permissions.append(n.text)
					u.remove(n)
			# set new permissions
			if ('/config' in permissions or '/config/users' in permissions):
				# user is administrator
				etree.SubElement(u, '{%s}Permission' % PION_NS).set('type', 'Admin')
			else:
				if ('/config/vocabularies' in permissions):
					etree.SubElement(u, '{%s}Permission' % PION_NS).set('type', 'Vocabularies')
				if ('/config/codecs' in permissions):
					etree.SubElement(u, '{%s}Permission' % PION_NS).set('type', 'Codecs')
				if ('/config/databases' in permissions):
					etree.SubElement(u, '{%s}Permission' % PION_NS).set('type', 'Databases')
				if ('/config/protocols' in permissions):
					etree.SubElement(u, '{%s}Permission' % PION_NS).set('type', 'Protocols')
				if ('/config/reactors' in permissions):
					p = etree.SubElement(u, '{%s}Permission' % PION_NS)
					p.set('type', 'Reactors')
					etree.SubElement(p, '{%s}Unrestricted' % PION_NS).text = 'true'
				if ('/replay' in permissions):
					p = etree.SubElement(u, '{%s}Permission' % PION_NS)
					p.set('type', 'ReplayService')
					etree.SubElement(p, '{%s}Unrestricted' % PION_NS).text = 'true'
		if (not QUIET and not TEST): print 'WARNING: Please use the web interface to review user permissions.'
	def add_new_workspace(self, cfg, workspace_id, workspace_name):
		# create a workspace node
		new_node = etree.Element('{%s}Workspace' % PION_NS)
		new_node.set('id', workspace_id)
		etree.SubElement(new_node, '{%s}Name' % PION_NS).text = workspace_name
		# add workspace node to doc tree
		cfg.root.insert(0, new_node)
	def update_reactors(self, cfg):
		reactor_nodes = list(cfg.root.iter('{%s}Reactor' % PION_NS))
		if (not reactor_nodes):
			# empty reactors.xml -> create default workspace
			self.add_new_workspace(cfg, str(uuid.uuid4()), 'Default')
			return
		# look for MDR reactors
		for r in reactor_nodes:
			plugin = r.findtext('{%s}Plugin' % PION_NS)
			if (plugin == 'MultiDatabaseReactor'):
				for tabledef in r.iter('{%s}TableDef' % PION_NS):
					table = tabledef.findtext('{%s}Table' % PION_NS)
					if (table == 'requests'):
						field = etree.SubElement(tabledef, '{%s}Field' % PION_NS)
						field.set('term', 'urn:vocab:clickstream#cs-content-type')
						field.text = 'cs_content_type'
						field = etree.SubElement(tabledef, '{%s}Field' % PION_NS)
						field.set('term', 'urn:vocab:clickstream#cs-content')
						field.text = 'cs_content'
		# look for all Workspace references
		workspaces = dict()
		workspace_nodes = list(cfg.root.iter('{%s}Workspace' % PION_NS))
		for w in reversed(workspace_nodes):
			# get name, check if we already have seen this workspace
			workspace_name = w.text
			workspace_id = workspaces.get(workspace_name, None)
			if (not workspace_id):
				# generate a random id, and update map
				workspace_id = str(uuid.uuid4())
				workspaces[workspace_name] = workspace_id
				# create a workspace node in XML doc
				self.add_new_workspace(cfg, workspace_id, workspace_name)
			# replace descriptive name with id
			w.text = workspace_id
	def add_new_service(self, server_node, type, id, name, comment, plugin, resource):
		n = etree.SubElement(server_node, '{%s}%s' % (PION_NS,type))
		n.set('id', id)
		etree.SubElement(n, '{%s}Name' % PION_NS).text = name
		etree.SubElement(n, '{%s}Comment' % PION_NS).text = comment
		etree.SubElement(n, '{%s}Plugin' % PION_NS).text = plugin
		etree.SubElement(n, '{%s}Resource' % PION_NS).text = resource
	def update_services(self, cfg):
		# remove entry for "log-service"
		server_node = None
		for n in cfg.root.iter('{%s}WebService' % PION_NS):
			if (n.get('id') == 'log-service'):
				server_node = n.getparent()
				server_node.remove(n)
		# add new service entries
		self.add_new_service(server_node, 'PlatformService',
			'monitor-service', 'Event Data Monitoring Service',
			'Pion platform event data monitoring service',
			'MonitorService', '/monitor')
		self.add_new_service(server_node, 'PlatformService',
			'xml-log-service', 'XML Log Service',
			'Recent Log entries in XML',
			'XMLLogService', '/xmllog')
	def update_protocols(self, cfg):
		# look for HTTPProtocol configs
		for n in cfg.root.iter('{%s}Protocol' % PION_NS):
			plugin = n.findtext('{%s}Plugin' % PION_NS)
			if (plugin == 'HTTPProtocol'):
				# find extraction rule for content-type
				for idx in range(0, len(n)):
					if (n[idx].tag=='{%s}Extract' % PION_NS and n[idx].get('term') == 'urn:vocab:clickstream#content-type'):
						break
				if (idx < len(n)):
					# insert cs-content-type before content-type
					extract = etree.Element('{%s}Extract' % PION_NS)
					extract.set('term', 'urn:vocab:clickstream#cs-content-type')
					etree.SubElement(extract, '{%s}Source' % PION_NS).text = 'cs-header'
					etree.SubElement(extract, '{%s}Name' % PION_NS).text = 'Content-Type'
					n.insert(idx, extract)
	def update_vocabs(self, cfg):
		# update vocabularies.xml
		v = etree.SubElement(cfg.root, '{%s}VocabularyConfig' % PION_NS)
		v.set('id', 'urn:vocab:omniture')
		v.text = 'vocabularies/omniture.xml'
	def insert_term(self, v, idx, id, type, comment):
		t = etree.Element('{%s}Term' % PION_NS)
		t.set('id', id)
		etree.SubElement(t, '{%s}Type' % PION_NS).text = type
		etree.SubElement(t, '{%s}Comment' % PION_NS).text = comment
		v.insert(idx, t)
	def replace_comment(self, term, comment):
		n = term.find('{%s}Comment' % PION_NS)
		if (n is not None):
			n.text = comment
		else:
			etree.SubElement(term, '{%s}Comment' % PION_NS).text = comment
	def update_clickstream(self, cfg):
		# update clickstream.xml
		for v in cfg.root.iter('{%s}Vocabulary' % PION_NS):
			for t in v.iter('{%s}Term' % PION_NS):
				if (t.get('id') == 'urn:vocab:clickstream#content-type'):
					self.insert_term(v, v.index(t), 'urn:vocab:clickstream#cs-content-type', 'string', 'The Content-Type HTTP request header')
				elif (t.get('id') == 'urn:vocab:clickstream#new-page'):
					self.insert_term(v, v.index(t), 'urn:vocab:clickstream#refused', 'uint32', 'Number of HTTP requests that were refused by the server')
					self.insert_term(v, v.index(t), 'urn:vocab:clickstream#canceled', 'uint32', 'Number of HTTP responses that were canceled early by the client')
				elif (t.get('id') == 'urn:vocab:clickstream#request-status'):
					self.replace_comment(t, 'HTTP request status (0=NONE, 1=TRUNCATED, 2=PARTIAL, 3=OK)')
				elif (t.get('id') == 'urn:vocab:clickstream#response-status'):
					self.replace_comment(t, 'HTTP response status (0=NONE, 1=TRUNCATED, 2=PARTIAL, 3=OK)')
				elif (t.get('id') == 'urn:vocab:clickstream#tcp-status'):
					self.replace_comment(t, 'TCP handshake status (0=OK, 1=RESET, 2=IGNORED)')
	def process(self, pion_config):
		self.update_users(pion_config['UserConfig'])
		self.update_reactors(pion_config['ReactorConfig'])
		self.update_services(pion_config['ServiceConfig'])
		self.update_protocols(pion_config['ProtocolConfig'])
		self.update_vocabs(pion_config['VocabularyConfig'])
		self.update_clickstream(pion_config.vocab['urn:vocab:clickstream'])

RULES.append(Upgrade30xTo31x('3.1.2', '^3\.0\..*$'))

# ...

#^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^#
# CONFIGURATION UPGRADE RULES GO ABOVE #
########################################

# global variables for Pion's XML namespaces
PION_NS = 'http://purl.org/pion/config'
NS_MAP = { None: PION_NS }

# misc global variables for logging
VERBOSE = False
QUIET = False
TEST = False

class XMLParseError(Exception):
	"""Class used to indicate an XML parsing error"""
	def __init__(self, value):
		self.value = value
	def __str__(self):
		return repr(self.value)

class XMLConfig(object):
	"""Class used to hold XML configuration data"""
	def __init__(self, file, name, rootname = None):
		self.name = name
		self.file = file
		if (VERBOSE): print self.name + ': parsing XML configuration file: ' + self.file
		parser = etree.XMLParser(remove_blank_text=True)
		self.tree = etree.parse(self.file, parser)
		if (not rootname):
			rootname = name
		self.root = self.tree.getroot()
		if (self.root.tag != ('{%s}%s' % (PION_NS,rootname)) ):
			raise XMLParseError('Root element mismatch (' + self.root.tag + '!=' + ('{%s}%s' % (PION_NS,rootname)) + ' in ' + self.file)
		self.version = self.root.get('pion_version')
		if (not self.version):
			raise XMLParseError('No version found in ' + self.file)
	def get(self, name):
		"""returns the value of an XML element"""
		return self.tree.find('{%s}%s' % (PION_NS,name)).text
	def backup(self, path):
		"""creates a backup of the configuration file"""
		if (VERBOSE): print self.name + ': backing up ' + self.file + ' to ' + path
		if (not TEST): shutil.copy(self.file, path)
	def save(self):
		"""saves updated configuration file"""
		if (VERBOSE): print self.name + ': saving ' + self.file
		if (not TEST):
			self.tree.write(self.file, pretty_print=True, encoding='UTF-8', xml_declaration='True')
	def set_version(self, version):
		"""updates the configuration file version"""
		self.version = version
		self.root.set('pion_version', self.version)

class PionConfig(dict):
	"""Class used to hold all Pion configuration data"""
	def __init__(self, config_path):
		self.config_path = config_path
		# start by parsing platform.xml in the config directory
		platform_cfg = self.parse('platform.xml', 'PionConfig')
		self.version = platform_cfg.version
		# parse the rest of the config files
		self.parse(platform_cfg.get('CodecConfig'), 'CodecConfig', 'PionConfig')
		self.parse(platform_cfg.get('DatabaseConfig'), 'DatabaseConfig', 'PionConfig')
		self.parse(platform_cfg.get('ReactorConfig'), 'ReactorConfig', 'PionConfig')
		self.parse(platform_cfg.get('ServiceConfig'), 'ServiceConfig', 'PionConfig')
		self.parse(platform_cfg.get('ProtocolConfig'), 'ProtocolConfig', 'PionConfig')
		self.parse(platform_cfg.get('UserConfig'), 'UserConfig', 'PionConfig')
		vocab_cfg = self.parse(platform_cfg.get('VocabularyConfig'), 'VocabularyConfig', 'PionConfig')
		# parse vocabulary configuration files
		self.vocab = dict()
		for v in vocab_cfg.root.iter('{%s}VocabularyConfig' % PION_NS):
			vocab_id = v.get('id')
			vocab_file = os.path.join(os.path.dirname(vocab_cfg.file), v.text)
			self.vocab[vocab_id] = XMLConfig(vocab_file, vocab_id, 'PionConfig')
		# these use different root elements
		self.parse('robots.xml', 'RobotConfig')
		self.parse('SearchEngines.xml', 'SearchEngineConfig')
		self.parse('ReplayQueries.xml', 'ReplayTemplates')
		self.parse('dbengines.xml', 'DatabaseTemplates')
		# other config options -> not XML files
		self.logconfig_file = os.path.join(self.config_path, platform_cfg.get('LogConfig'))
		self.plugin_path = os.path.join(self.config_path, platform_cfg.get('PluginPath'))
		self.data_dir = os.path.join(self.config_path, platform_cfg.get('DataDirectory'))
		if (VERBOSE):
			print 'log configuration file: ' + self.logconfig_file
			print 'path to plugin files: ' + self.plugin_path
			print 'data directory: ' + self.data_dir
			print 'current version: ' + self.version
	def parse(self, file, name, rootname = None):
		"""parses a standard Pion XML configuration file"""
		cfg = XMLConfig(os.path.join(self.config_path, file), name, rootname)
		self[name] = cfg
		return cfg
	def backup(self, backup_path):
		"""creates a backup of all configuration files"""
		if (not QUIET): print 'creating backups in ' + backup_path
		if (not TEST and not os.path.exists(backup_path)):
			os.makedirs(backup_path)
		for name, cfg in self.items():
			cfg.backup(backup_path)
		# backup vocabulary configuration files
		vocab_path = os.path.join(backup_path, 'vocabularies')
		if (not TEST and not os.path.exists(vocab_path)):
			os.makedirs(vocab_path)
		for name, cfg in self.vocab.items():
			cfg.backup(vocab_path)
	def save(self):
		"""saves updated configuration file"""
		if (not QUIET): print 'saving configuration files'
		for name, cfg in self.items():
			cfg.save()
		for name, cfg in self.vocab.items():
			cfg.save()
	def set_version(self, version):
		"""updates the configuration file version"""
		self.version = version
		for name, cfg in self.items():
			cfg.set_version(self.version)
		for name, cfg in self.vocab.items():
			cfg.set_version(self.version)
	def update_paths(self, new_path):
		"""updates paths for all configuration files"""
		vocab_path = os.path.join(new_path, 'vocabularies')
		if (not TEST and not os.path.exists(new_path)):
			os.makedirs(new_path)
		if (not TEST and not os.path.exists(vocab_path)):
			os.makedirs(vocab_path)
		for name, cfg in self.items():
			cfg.file = os.path.join(new_path, os.path.basename(cfg.file))
		for name, cfg in self.vocab.items():
			cfg.file = os.path.join(vocab_path, os.path.basename(cfg.file))
	

def parse_args():
	"""parses and returns command-line arguments"""
	# prepare argument parser
	parser = optparse.OptionParser(usage="usage: %prog [options] configpath")
	parser.add_option("-o", "--output", action="store", default=None,
		help="save updated configuration in this directory")
	parser.add_option("-t", "--test", action="store_true", default=False,
		help="test run only, do not modify (default=False)")
	parser.add_option("-v", "--verbose", action="store_true", default=False,
		help="enables verbose operation (default=False)")
	parser.add_option("-q", "--quiet", action="store_true", default=False,
		help="enables quiet operation (default=False)")
	# parse command-line arguments
	options, arguments = parser.parse_args()		
	# check validity of arguments
	if (len(arguments) != 1):
		print 'error: No configuration path argument was specified'
		sys.exit(1)
	options.config_path = arguments[0]
	return options


def main():
	"""main processing function"""
	global VERBOSE
	global QUIET
	global TEST
	# parse command-line options
	options = parse_args()
	VERBOSE = options.verbose
	QUIET = options.quiet
	TEST = options.test
	# get configuration
	config = PionConfig(options.config_path)
	current_version = config.version
	# process upgrade rules
	upgraded = False
	for r in RULES:
		# process a rule only if version patches pattern
		if (re.search(r.regex, config.version)):
			r.start(config)
			r.process(config)
			r.finish(config)
			upgraded = True
	if (upgraded):
		# config was updated
		if (options.output):
			# save config to new output directory
			config.update_paths(options.output)
		else:
			# make backup and overwrite config
			config.backup(os.path.join(config.config_path, 'backup-' + current_version))
		config.save()
	else:
		if (not QUIET): print 'configuration is up-to-date (' + current_version + ')'


# call main() if script is being executed	
if __name__ == '__main__':
	main()
