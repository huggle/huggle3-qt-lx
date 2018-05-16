//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

function alert(message)
{
    if (huggle_context() === "huggle_ui")
    {
        huggle_ui_message_box("alert", message);
    } else
    {
        huggle_log(message);
    }
}

function console() {}
console.assert = function(eval, txt) { if (eval) { huggle_log(txt); } }
console.log = function(txt) { huggle_log(txt); }
console.error = function(txt) { huggle_error_log(txt); }
console.debug = function(txt) { huggle_debug_log(txt, 1); }
console.warn = function(txt) { huggle_warning_log(txt); }
