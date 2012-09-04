function printMenu(thePage)
{
    document.write("    <div class=\"header\"><div class=\"page\">\n");
    document.write("      <div class=\"menu\">\n");
    printMenuLinks(thePage);
    document.write("      </div>\n");
    document.write("      <img src=\"images/Vintage-title.png\" alt=\"Vintage\" />\n");
    document.write("    </div></div>\n");
}

/*
 * page could be "news", "history" or "downloads"
 */
function printMenuLinks(selectedPage)
{
	printMenuLink("news", "Новости", selectedPage);
	document.write("          <span class=\"divider\"> | </span>"); 
	printMenuLink("documentation", "Документация", selectedPage);
	document.write("          <span class=\"divider\"> | </span>"); 
	printMenuLink("downloads", "Скачать", selectedPage);
}

function printMenuLink(linkName, linkText, selectedPage)
{
	if (selectedPage == linkName)
		document.write("          <span class=\"item-selected\">" + linkText + "</span>");
	else
		document.write("          <a class=\"item\" href=\"" + linkName + ".html\">" + linkText + "</a>"); 
}