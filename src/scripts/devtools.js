// Helper script for devs

function ext_is_working()
{
    return true;
}

function ext_get_desc()
{
    return "Developer tools";
}

function ext_get_name()
{
    return "devtools";
}

function ext_get_version()
{
    return "1.0.0";
}

function ext_get_author()
{
    return "Petr Bena";
}

function print_help()
{
    var help = "<html>\n";
    help += "<body>\n";
    help += "<h1>Huggle developer manual</h1>\n";
    help += "<p>This page is a reference manual for all functions and hooks available in scripting engine:</p>\n";
    help += "<h2>Functions</h2>\n";
    help += "<table>\n";
    help += "<tr><th>Function</th></tr>\n";
    var functions = huggle_get_function_list();
    for (i = 0, len = functions.length; i < len; i++)
    {
        var function_name = functions[i];
        help += "<tr><td>" + function_name + huggle_get_function_help(function_name) + "</td>\n";
    }
    help += "</table>\n";
    help += "<h2>Hooks</h2>\n";
    help += "<table>\n";
    help += "<tr><th>Name</th></tr>\n";
    functions = huggle_get_hook_list();
    for (i = 0, len = functions.length; i < len; i++)
    {
        var function_name = functions[i];
        help += "<tr><td>" + function_name + huggle_get_function_help(function_name) + "</td>\n";
    }
    help += "</table>\n";
    help += "</body>\n";
    help += "</html>";
    huggle_ui_render_html(help, true);
}

function ext_init()
{
    return true;
}

function ext_on_main_open()
{
    huggle_ui_create_menu(huggle_ui_menu_help, "Show scipting manual", "print_help");
}
