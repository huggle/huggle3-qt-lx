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
    help += "<head>\n";
    help += "<style>* { font-family: Helvetica Neue, Helvetica, Arial, sans-serif; }</style>\n";
    help += "</head>\n";
    help += "<body>\n";
    help += "<h1>Huggle developer manual</h1>\n";
    help += "<p>Huggle version: " + huggle.get_version()["String"] + ", unsafe functions enabled: " + huggle.is_unsafe() + ", context: " + huggle.get_context() + "</p>\n";
    help += "<p>This page is a reference manual for all functions and hooks available in scripting engine:</p>\n";
    help += "<h2>Functions</h2>\n";
    help += "<table>\n";
    help += "<tr><th>Function</th></tr>\n";
    var functions = huggle.get_function_list();
    for (i = 0, len = functions.length; i < len; i++)
    {
        var function_name = functions[i];
        help += "<tr><td><b>" + function_name + "</b>" + huggle.get_function_help(function_name) + "</td>\n";
    }
    help += "</table>\n";
    help += "<h2>Ecma functions</h2>\n";
    help += "<table>\n";
    help += "<tr><th>Function</th></tr>\n";
    functions = huggle_ecma_function_list;
    for (i = 0, len = functions.length; i < len; i++)
    {
        var function_name = functions[i];
        help += "<tr><td><b>" + function_name + "</b>" + huggle_ecma_function_help[function_name] + "</td>\n";
    }
    help += "<table>\n";
    help += "<h2>Hooks</h2>\n";
    help += "<table>\n";
    help += "<tr><th>Name</th></tr>\n";
    functions = huggle.get_hook_list();
    for (i = 0, len = functions.length; i < len; i++)
    {
        var function_name = functions[i];
        help += "<tr><td><b>" + function_name + "</b>" + huggle.get_function_help(function_name) + "</td>\n";
    }
    help += "</table>\n";
    help += "</body>\n";
    help += "</html>";
    huggle_ui.render_html(help, true);
}

function ext_init()
{
    // Register hook to run when main window is loaded
    if (!huggle.register_hook("main_open", "ext_on_main_open"))
        return false;

    return true;
}

function ext_on_main_open()
{
    huggle_ui.create_menu(huggle_ui_menu_help, "Show scipting manual", "print_help");
}
