
<ul id="nav">
<li><span>Browse by Category</li>
{foreach key=id item=name from=$cats}
<li><a href="./search.php?cat={$id}">{$name}</a></li>
{/foreach}
</ul>

