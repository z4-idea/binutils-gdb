/* CLI colorizing

   Copyright (C) 2018-2019 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "defs.h"
#include "cli/cli-cmds.h"
#include "cli/cli-style.h"
#include "source-cache.h"

/* True if styling is enabled.  */

#if defined(_WIN32) || defined (__CYGWIN__)
int cli_styling = 0;
#else
int cli_styling = 1;
#endif

/* Name of colors; must correspond to ui_file_style::basic_color.  */
static const char * const cli_colors[] = {
  "none",
  "black",
  "red",
  "green",
  "yellow",
  "blue",
  "magenta",
  "cyan",
  "white",
  nullptr
};

/* Names of intensities; must correspond to
   ui_file_style::intensity.  */
static const char * const cli_intensities[] = {
  "normal",
  "bold",
  "dim",
  nullptr
};

/* See cli-style.h.  */

cli_style_option file_name_style (ui_file_style::GREEN);

/* See cli-style.h.  */

cli_style_option function_name_style (ui_file_style::YELLOW);

/* See cli-style.h.  */

cli_style_option variable_name_style (ui_file_style::CYAN);

/* See cli-style.h.  */

cli_style_option address_style (ui_file_style::BLUE);

/* See cli-style.h.  */

cli_style_option::cli_style_option (ui_file_style::basic_color fg)
  : m_foreground (cli_colors[fg - ui_file_style::NONE]),
    m_background (cli_colors[0]),
    m_intensity (cli_intensities[ui_file_style::NORMAL])
{
}

/* Return the color number corresponding to COLOR.  */

static int
color_number (const char *color)
{
  for (int i = 0; i < ARRAY_SIZE (cli_colors); ++i)
    {
      if (color == cli_colors[i])
	return i - 1;
    }
  gdb_assert_not_reached ("color not found");
}

/* See cli-style.h.  */

ui_file_style
cli_style_option::style () const
{
  int fg = color_number (m_foreground);
  int bg = color_number (m_background);
  ui_file_style::intensity intensity = ui_file_style::NORMAL;

  for (int i = 0; i < ARRAY_SIZE (cli_intensities); ++i)
    {
      if (m_intensity == cli_intensities[i])
	{
	  intensity = (ui_file_style::intensity) i;
	  break;
	}
    }

  return ui_file_style (fg, bg, intensity);
}

/* See cli-style.h.  */

void
cli_style_option::do_set (const char *args, int from_tty)
{
}

/* See cli-style.h.  */

void
cli_style_option::do_show (const char *args, int from_tty)
{
}

/* See cli-style.h.  */

void
cli_style_option::do_show_foreground (struct ui_file *file, int from_tty,
				      struct cmd_list_element *cmd,
				      const char *value)
{
  const char *name = (const char *) get_cmd_context (cmd);
  fprintf_filtered (file, _("The \"%s\" foreground color is: %s\n"),
		    name, value);
}

/* See cli-style.h.  */

void
cli_style_option::do_show_background (struct ui_file *file, int from_tty,
				      struct cmd_list_element *cmd,
				      const char *value)
{
  const char *name = (const char *) get_cmd_context (cmd);
  fprintf_filtered (file, _("The \"%s\" background color is: %s\n"),
		    name, value);
}

/* See cli-style.h.  */

void
cli_style_option::do_show_intensity (struct ui_file *file, int from_tty,
				     struct cmd_list_element *cmd,
				     const char *value)
{
  const char *name = (const char *) get_cmd_context (cmd);
  fprintf_filtered (file, _("The \"%s\" display intensity is: %s\n"),
		    name, value);
}

/* See cli-style.h.  */

void
cli_style_option::add_setshow_commands (const char *name,
					enum command_class theclass,
					const char *prefix_doc,
					const char *prefixname,
					struct cmd_list_element **set_list,
					struct cmd_list_element **show_list)
{
  m_set_prefix = std::string ("set ") + prefixname + " ";
  m_show_prefix = std::string ("show ") + prefixname + " ";

  add_prefix_cmd (name, no_class, do_set, prefix_doc, &m_set_list,
		  m_set_prefix.c_str (), 0, set_list);
  add_prefix_cmd (name, no_class, do_show, prefix_doc, &m_show_list,
		  m_show_prefix.c_str (), 0, show_list);

  add_setshow_enum_cmd ("foreground", theclass, cli_colors,
			&m_foreground,
			_("Set the foreground color for this property"),
			_("Show the foreground color for this property"),
			nullptr,
			nullptr,
			do_show_foreground,
			&m_set_list, &m_show_list, (void *) name);
  add_setshow_enum_cmd ("background", theclass, cli_colors,
			&m_background,
			_("Set the background color for this property"),
			_("Show the background color for this property"),
			nullptr,
			nullptr,
			do_show_background,
			&m_set_list, &m_show_list, (void *) name);
  add_setshow_enum_cmd ("intensity", theclass, cli_intensities,
			&m_intensity,
			_("Set the display intensity color for this property"),
			_("\
Show the display intensity color for this property"),
			nullptr,
			nullptr,
			do_show_intensity,
			&m_set_list, &m_show_list, (void *) name);
}

static void
set_style (const char *arg, int from_tty)
{
}

static void
show_style (const char *arg, int from_tty)
{
}

static void
set_style_enabled  (const char *args, int from_tty, struct cmd_list_element *c)
{
  g_source_cache.clear ();
}

static void
show_style_enabled (struct ui_file *file, int from_tty,
		    struct cmd_list_element *c, const char *value)
{
  if (cli_styling)
    fprintf_filtered (file, _("CLI output styling is enabled.\n"));
  else
    fprintf_filtered (file, _("CLI output styling is disabled.\n"));
}

void
_initialize_cli_style ()
{
  static cmd_list_element *style_set_list;
  static cmd_list_element *style_show_list;

  add_prefix_cmd ("style", no_class, set_style, _("\
Style-specific settings\n\
Configure various style-related variables, such as colors"),
		  &style_set_list, "set style ", 0, &setlist);
  add_prefix_cmd ("style", no_class, show_style, _("\
Style-specific settings\n\
Configure various style-related variables, such as colors"),
		  &style_show_list, "show style ", 0, &showlist);

  add_setshow_boolean_cmd ("enabled", no_class, &cli_styling, _("\
Set whether CLI styling is enabled."), _("\
Show whether CLI is enabled."), _("\
If enabled, output to the terminal is styled."),
			   set_style_enabled, show_style_enabled,
			   &style_set_list, &style_show_list);

  file_name_style.add_setshow_commands ("filename", no_class,
					_("\
Filename display styling\n\
Configure filename colors and display intensity."),
					"style filename",
					&style_set_list,
					&style_show_list);
  function_name_style.add_setshow_commands ("function", no_class,
					    _("\
Function name display styling\n\
Configure function name colors and display intensity"),
					    "style function",
					    &style_set_list,
					    &style_show_list);
  variable_name_style.add_setshow_commands ("variable", no_class,
					    _("\
Variable name display styling\n\
Configure variable name colors and display intensity"),
					    "style variable",
					    &style_set_list,
					    &style_show_list);
  address_style.add_setshow_commands ("address", no_class,
				      _("\
Address display styling\n\
Configure address colors and display intensity"),
				      "style address",
				      &style_set_list,
				      &style_show_list);
}
