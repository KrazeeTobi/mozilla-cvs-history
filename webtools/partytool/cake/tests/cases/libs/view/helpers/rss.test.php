<?php
/* SVN FILE: $Id: rss.test.php,v 1.1 2007/11/19 09:16:18 rflint%ryanflint.com Exp $ */
/**
 * Short description for file.
 *
 * Long description for file
 *
 * PHP versions 4 and 5
 *
 * CakePHP(tm) Tests <https://trac.cakephp.org/wiki/Developement/TestSuite>
 * Copyright 2005-2007, Cake Software Foundation, Inc.
 *								1785 E. Sahara Avenue, Suite 490-204
 *								Las Vegas, Nevada 89104
 *
 *  Licensed under The Open Group Test Suite License
 *  Redistributions of files must retain the above copyright notice.
 *
 * @filesource
 * @copyright		Copyright 2005-2007, Cake Software Foundation, Inc.
 * @link				https://trac.cakephp.org/wiki/Developement/TestSuite CakePHP(tm) Tests
 * @package			cake.tests
 * @subpackage		cake.tests.cases.libs.view.helpers
 * @since			CakePHP(tm) v 1.2.0.4206
 * @version			$Revision: 1.1 $
 * @modifiedby		$LastChangedBy: phpnut $
 * @lastmodified	$Date: 2007/11/19 09:16:18 $
 * @license			http://www.opensource.org/licenses/opengroup.php The Open Group Test Suite License
 */
if (!defined('CAKEPHP_UNIT_TEST_EXECUTION')) {
	define('CAKEPHP_UNIT_TEST_EXECUTION', 1);
}

require_once CAKE.'app_helper.php';
uses('controller'.DS.'controller', 'model'.DS.'model', 'view'.DS.'helper', 'view'.DS.'helpers'.DS.'rss');

/**
 * Short description for class.
 *
 * @package		cake.tests
 * @subpackage	cake.tests.cases.libs.view.helpers
 */
class RssTest extends UnitTestCase {

	function setUp() {
		$this->Rss = new RssHelper();
	}

	function tearDown() {
		unset($this->Rss);
	}

	function testDocument() {
		$res = $this->Rss->document();
		$this->assertPattern('/^<rss version="2.0" \/>$/', $res);

		$res = $this->Rss->document(array('contrived' => 'parameter'));
		$this->assertPattern('/^<rss version="2.0"><\/rss>$/', $res);

		$res = $this->Rss->document(null, 'content');
		$this->assertPattern('/^<rss version="2.0">content<\/rss>$/', $res);

		$res = $this->Rss->document(array('contrived' => 'parameter'), 'content');
		$this->assertPattern('/^<rss[^<>]+version="2.0"[^<>]*>/', $res);
		$this->assertPattern('/<rss[^<>]+contrived="parameter"[^<>]*>/', $res);
		$this->assertNoPattern('/<rss[^<>]+[^version|contrived]=[^<>]*>/', $res);
	}

	function testChannel() {
		$attrib = array('a' => '1', 'b' => '2');
		$elements['title'] = 'title';
		$content = 'content';
		$res = $this->Rss->channel($attrib, $elements, $content);
		$this->assertPattern('/^<channel[^<>]+a="1"[^<>]*>/', $res);
		$this->assertPattern('/^<channel[^<>]+b="2"[^<>]*>/', $res);
		$this->assertNoPattern('/^<channel[^<>]+[^a|b]=[^<>]*/', $res);
		$this->assertPattern('/<title>title<\/title>/', $res);
		$this->assertPattern('/<link>'.str_replace('/', '\/', RssHelper::url('/', true)).'<\/link>/', $res);
		$this->assertPattern('/<description \/>/', $res);
		$this->assertPattern('/content<\/channel>$/', $res);
	}

	function testChannelElementLevelAttrib() {
		$attrib = array();
		$elements['title'] = 'title';
		$elements['image'] = array('myImage', 'attrib' => array('href' => 'http://localhost'));
		$content = 'content';
		$res = $this->Rss->channel($attrib, $elements, $content);
		$this->assertPattern('/^<channel>/', $res);
		$this->assertPattern('/<title>title<\/title>/', $res);
		$this->assertPattern('/<image[^<>]+href="http:\/\/localhost">myImage<\/image>/', $res);
		$this->assertPattern('/<link>'.str_replace('/', '\/', RssHelper::url('/', true)).'<\/link>/', $res);
		$this->assertPattern('/<description \/>/', $res);
		$this->assertPattern('/content<\/channel>$/', $res);
	}

	function testItems() {
	}

	function testItem() {
		$result = $this->Rss->item(null, array("title"=>"My title","description"=>"My description","link"=>"http://www.google.com/"));
		$expecting = '<item><title>My title</title><description>My description</description><link>http://www.google.com/</link><guid>http://www.google.com/</guid></item>';
		$this->assertEqual($result, $expecting);
	}

	function testTime() {
	}

	function testElementAttrNotInParent() {
		$attributes = array('title' => 'Some Title', 'link' => 'http://link.com', 'description' => 'description');
		$elements = array('enclosure' => array('url' => 'http://somewhere.com'));

		$result = $this->Rss->item($attributes, $elements);
		$this->assertPattern('/^<item title="Some Title" link="http:\/\/link.com" description="description"><enclosure url="http:\/\/somewhere.com" \/><\/item>$/', $result);
	}
}
?>