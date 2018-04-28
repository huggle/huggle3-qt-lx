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

}

function ext_init()
{
    return true;
}

function ext_on_main_open()
{
    huggle_ui_create_menu(huggle_ui_menu_help, "Show scipting manual", "print_help");
}
