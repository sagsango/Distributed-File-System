/*
	$Id$

	Frontend for starting and stopping captures


	Copyright (C) 2007 Alex Waugh
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "rtk/desktop/application.h"
#include "rtk/desktop/menu_item.h"
#include "rtk/desktop/menu.h"
#include "rtk/desktop/info_dbox.h"
#include "rtk/desktop/ibar_icon.h"
#include "rtk/desktop/label.h"
#include "rtk/desktop/writable_field.h"
#include "rtk/desktop/action_button.h"
#include "rtk/desktop/default_button.h"
#include "rtk/desktop/grid_layout.h"
#include "rtk/desktop/row_layout.h"
#include "rtk/desktop/column_layout.h"
#include "rtk/events/menu_selection.h"
#include "rtk/events/close_window.h"
#include "rtk/events/user_drag_box.h"
#include "rtk/events/datasaveack.h"
#include "rtk/transfer/save.h"
#include "rtk/os/wimp.h"


using namespace rtk::desktop;

class salmonsave:
	public rtk::transfer::save
{
public:
	salmonsave() { allow_ram_transfer(false); }
	void handle_event(rtk::events::datasaveack& ev);
	void start(void) {}
	void get_block(const void**, unsigned *) {}
	void finish(void) {}
	unsigned estsize(void) { return 0; }
};

class wiresalmon:
	public rtk::desktop::application,
	public rtk::events::menu_selection::handler,
	public rtk::events::user_drag_box::handler,
	public rtk::events::mouse_click::handler
{
public:
	wiresalmon();
	void handle_event(rtk::events::menu_selection& ev);
	void handle_event(rtk::events::mouse_click& ev);
	void handle_event(rtk::events::user_drag_box& ev);
	writable_field pathname;
	void start_capture(const char *filename);
	void stop_capture(void);
	void kill_module(void);
private:
	ibar_icon ibicon;
	menu ibmenu;
	menu_item ibinfo;
	menu_item ibhelp;
	menu_item ibquit;
	prog_info_dbox proginfo;

	window win;

	action_button start;
	action_button stop;
	icon filetype;

	column_layout layout1;
	column_layout layout2;
	row_layout layout3;

	salmonsave saveop;
};

