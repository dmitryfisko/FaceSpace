var list_across0 = [
'_contents_xml.htm',
'_reference.xml',
'_index.xml',
'_search_xml.htm',
'_external.xml'
];
var list_up0 = [
'mat2cpp.xml',
'news.xml'
];
var list_down1 = [
'license.xml',
'getstarted.xml',
'operation.xml',
'mat2cpp.hpp.xml',
'mat2cpp_ok.m.xml',
'mat2cpp_ok.cpp.xml',
'news.xml',
'library.xml',
'mztuni.xml'
];
var list_current0 = [
'news.xml#2013-07-25',
'news.xml#2013-07-20',
'news.xml#06-05-15',
'news.xml#06-05-05',
'news.xml#06-04-18'
];
function choose_across0(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_across0[index-1];
}
function choose_up0(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_up0[index-1];
}
function choose_down1(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_down1[index-1];
}
function choose_down0(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_down0[index-1];
}
function choose_current0(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_current0[index-1];
}
