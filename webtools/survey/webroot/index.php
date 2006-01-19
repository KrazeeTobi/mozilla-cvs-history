<?php
/**
 * Uninstall survey.
 * @package survey
 * @subpackage docs
 */
$intends = $app->getIntends();
$issues = $app->getIssues();

if (empty($_POST['submit'])) {
echo '<form action="./" method="post">';

// Create intend block.
echo '<h2>How did you intend to use Firefox when you installed it?</h2>';
echo '<ul>';
foreach ($intends as $id=>$text) {
    echo '<li><input type="radio" name="intend_id" id="int'.$id.'" value="'.$id.'" /> <label for="int'.$id.'">'.$text.'</label></li>';
}
echo '<li><input type="radio" name="intend_id" id="int0" value="0" /> <label for="intother" onclick="getElementById(\'int0\').checked=true;">Other, please specify: </label> <input type="text" name="other" id="intother" /></li>';
echo '</ul>';

// Create issue block.
echo '<h2>What issues, if any, did you have? (select all that apply)</h2>';
echo '<ul>';
foreach ($issues as $id=>$text) {
    echo '<li><input type="checkbox" name="issue_id[]" id="iss'.$id.'" value="'.$id.'" /> <label for="iss'.$id.'">'.$text.'</label></li>';
}
echo '</ul>';

echo '<h2>Other comments or suggestions?</h2>';
echo '<textarea name="comments"></textarea>';


echo '<div><input name="submit" type="submit" class="submit" value="Submit"/></div>';
echo '</form>';
}
?>
