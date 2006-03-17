<?php
/**
 * Global initialization script.
 *
 * @package amo
 * @subpackage docs
 */



/**
 * Require our config.
 */
require_once('config.php');



/**
 * Set runtime options.
 */
ini_set('display_errors',DISPLAY_ERRORS);
ini_set('error_reporting',ERROR_REPORTING);
ini_set('magic_quotes_gpc',0);



/**
 * Name of the current script.
 *
 * This is used in important comparisons with $shadow_config and $cache_config,
 * so it was pulled out of config.php so it's immutable.
 */
define('SCRIPT_NAME',substr($_SERVER['SCRIPT_NAME'], strlen(WEB_PATH.'/'), strlen($_SERVER['SCRIPT_NAME'])));



/**
 * Page output caching.
 *
 * First we will create a memcache object.
 * Second, we will check to see if the cache has already been compiled for our ID.
 * This is all done before anything else happens because we want to save cycles.
 */

/**
 * Set our memcacheId based on our params.
 * We'll keep it around even if we aren't using the cache.
 */
$memcacheId = md5(SCRIPT_NAME.$_SERVER['QUERY_STRING']);

/**
 * Instantiate our memcache object.
 */
$cache = new memcache();

/**
 * Check the cache_config to see if the current page is supposed to be cached.
 * If it is, try to connect.  If connection to the cache server fails, fall back on regular runtime.
 * We also are storing the status of the connection in $memcacheConnected for use in finish.php.
 */
if (!empty($cache_config[SCRIPT_NAME]) ) {
    
    $memcacheConnected = false;

    if (is_array($memcache_config)) {
        foreach ($memcache_config as $host=>$options) {
            if ($cache->addServer($host, $options['port'], $options['persistent'], $options['weight'], $options['timeout'], $options['retry_interval'])) {
                $memcacheConnected = true;
            }
        }
    }

    if ($memcacheConnected) {

        // If our page is already cached, display from cache and exit.
        if ($cacheData = $cache->get($memcacheId)) {

            // If we have specified a custom content-type for this script, echo the header here.
            if (!empty($contentType_config[SCRIPT_NAME])) {
                header('Content-type: '.$contentType_config[SCRIPT_NAME]);
            }

            echo $cacheData;
            exit;
        }
    } else {

	/**
         * If we get here, it means we were supposed to read from cache, but couldn't connect to memcached.
	 * Log the message, then continue with runtime.
         */
        error_log('Memcache Error: Unable connect to memcache server.  Please check configuration and try again.'); 
    }
}



/**
 * Set up global variables.
 */
$tpl = null;  // Smarty.
$content = null;  // Name of .tpl file to be called for content.
$pageType = null;  // Wrapper type for wrapping content .tpl.
$cacheId = null;  // Name of the page cache for use with Smarty caching.
$currentTab = null;  // Name of the currentTab selected in the header.tpl.  This is also the cacheId for header.tpl.
$compileId = null;  // Name of the compileId for use with Smarty caching.
$clean = array();  // General array for verified inputs.  These variables should not be formatted for a specific medium.
$sql = array();  // Trusted for SQL.  Items in this array must be trusted through logic or through escaping.

/**
 * CompileId is used for the compiled template generated by Smarty.
 * If app is not a valid format, not set or just empty, set it to firefox.
 */
$compileId = (!empty($_GET['app']) && ctype_alpha($_GET['app'])) ? $_GET['app'] : 'firefox';



/**
 * Smarty configuration.
 *
 * Include Smarty class.  If we get here, we're going to generate the page.
 */
require_once(SMARTY_BASEDIR.'/Smarty.class.php');  // Smarty



/**
 * Smarty configuration.
 */
class AMO_Smarty extends Smarty
{
    function AMO_Smarty()
    {
        $this->template_dir = TEMPLATE_DIR;
        $this->compile_dir = COMPILE_DIR;
        $this->cache_dir = CACHE_DIR;
        $this->config_dir = CONFIG_DIR;

        // Pass config variables to Smarty object.
        $this->assign('config',
            array(  'webpath'   => WEB_PATH,
                    'rootpath'  => ROOT_PATH,
                    'host'      => HTTP_HOST)
        );
    }
}



/**
 * Begin template processing.
 *
 * @param string $aTplName name of the template to process
 * @param string $aCacheId name of the cache to check - this corresponds to other $_GET params
 * @param string $aCompileId name of the compileId to check - this is $_GET['app']
 * @param string $aPageType type of the page - nonav, default, etc.
 */
function startProcessing($aTplName, $aCacheId, $aCompileId, $aPageType='default')
{
    // Pass in our global variables.
    global $tpl, $pageType, $content, $cacheId, $compileId, $cache_config;

    $pageType = $aPageType;
    $content = $aTplName;
    $cacheId = $aCacheId;
    $compileId = $aCompileId;

    $tpl = new AMO_Smarty();
}
?>
