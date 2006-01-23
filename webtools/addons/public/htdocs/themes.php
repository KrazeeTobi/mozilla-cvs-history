<?php
/**
 * Home page for extensions, switchable on application.
 *
 * @package amo
 * @subpackage docs
 */

$currentTab = 'themes';

startProcessing('themes.tpl', null, $compileId, 'nonav');
require_once('includes.php');

// If app is not set or empty, set it to null for our switch.
$_GET['app'] = (!empty($_GET['app'])) ? $_GET['app'] : null;

// Determine our application.
switch( $_GET['app'] ) {
    case 'mozilla':
        $clean['app'] = 'Mozilla';
        break;
    case 'thunderbird':
        $clean['app'] = 'Thunderbird';
        break;
    case 'firefox':
    default:
        $clean['app'] = 'Firefox';
        break;
}

// $sql['app'] can equal $clean['app'] since it was assigned in a switch().
// We have to ucfirst() it because the DB has caps.
$sql['app'] = $clean['app'];

// Get most popular extensions based on application.
$db->query("
    SELECT DISTINCT
        TM.ID ID, 
        TM.Name name, 
        TM.downloadcount dc
    FROM
        main TM
    INNER JOIN version TV ON TM.ID = TV.ID
    INNER JOIN applications TA ON TV.AppID = TA.AppID
    INNER JOIN os TOS ON TV.OSID = TOS.OSID
    WHERE
        AppName = '{$sql['app']}' AND 
        downloadcount > '0' AND
        approved = 'YES' AND
        Type = 'E'
    ORDER BY
        downloadcount DESC 
    LIMIT 
        5 
", SQL_ALL, SQL_ASSOC);

$popularExtensions = $db->record;


// Assign template variables.
$tpl->assign(
    array(  'popularExtensions' => $popularExtensions,
            'title'             => $clean['app'].' Addons',
            'currentTab'        => $currentTab)
);
?>
