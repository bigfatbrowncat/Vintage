var names = new Object();
names["about"] = "О проекте";
names["about/thoughts"] = "Размышления";
names["doc"] = "Документация";
names["downloads"] = "Скачать";

var htmls = new Object();
htmls["about"] = "about.html";
htmls["about/thoughts"] = "thoughts.html";
htmls["doc"] = "doc.html";
htmls["downloads"] = "downloads.html";

Object.size = function(obj) {
    var size = 0, key;
    for (key in obj) {
        if (obj.hasOwnProperty(key)) size++;
    }
    return size;
};


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
	splited = selectedPage.split("/");
	printMenuLink("about", selectedPage);
	document.write("          <span class=\"divider\"> | </span>"); 
	printMenuLink("doc", selectedPage);
	document.write("          <span class=\"divider\"> | </span>"); 
	printMenuLink("downloads", selectedPage);
}

function printMenuLink(baseName, selectedPage)
{
	splitted = selectedPage.split("/");

	if (splitted[0] != baseName)
	{
		document.write("<a class=\"item\" href=\"" + htmls[baseName] + "\">" + names[baseName] + "</a>");
	}
	else if (splitted.length == 1)
	{
		document.write("<span class=\"item-selected\">" + names[baseName] + "</span>");
	}
	else
	{
		path = splitted[0];
		document.write("          <a class=\"item\" href=\"" + htmls[path] + "\">" + names[path] + "</a>");

		for (i = 1; i < splitted.length - 1; i++)
		{
			path += "/" + splitted[i];
			document.write("<span class=\"divider\"> / </span><a class=\"item\" href=\"" + htmls[path] + "\">" + names[path] + "</a>");
		}
	
		if (splitted.length > 1)
		{
			path += "/" + splitted[splitted.length - 1];
			document.write("<span class=\"divider\"> / </span><span class=\"item-selected\">" + names[path] + "</span>");
		}
	}
}