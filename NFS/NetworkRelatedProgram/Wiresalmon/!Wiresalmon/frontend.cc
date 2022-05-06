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
#include "rtk/events/null_reason.h"
#include "rtk/os/wimp.h"
#include "rtk/os/os.h"
#include "rtk/swi/wimp.h"

#include "frontend.h"
#include "wiresalmonmod.h"

using namespace rtk;

wiresalmon::wiresalmon():
	application("Wiresalmon")
{
	proginfo.add("Name", "Wiresalmon");
	proginfo.add("Purpose", "Capture network packets");
	proginfo.add("Author", "© Alex Waugh, 2007");
	proginfo.add("Version", Module_VersionString " (" Module_Date ")");
	ibinfo.text("Info");
	ibinfo.attach_dbox(proginfo);
	ibhelp.text("Help...");
	ibquit.text("Quit");
	ibmenu.title("Wiresalmon");
	ibmenu.add(ibinfo);
	ibmenu.add(ibhelp);
	ibmenu.add(ibquit);
	ibicon.sprite_name("!Wiresalmon").hcentre(true);
	ibicon.attach_menu(ibmenu);
	add(ibicon);

	stop.text("Stop capture");
	stop.enabled(false);
	start.text("Start capture");

	layout1.add(start);
	layout1.add(stop);
	layout1.margin(16).ygap(8);

	win.title("Wiresalmon");

	filetype.sprite_name("file_ffd");
	filetype.button(6);
	filetype.xfit(false);
	filetype.yfit(false);
	filetype.xbaseline(xbaseline_centre);

	pathname.text("capture/pcap",255);
	pathname.validation(pathname.validation()+";A~ ");
	pathname.min_size(point(200,0));

	layout2.add(filetype);
	layout2.add(pathname);
	layout2.margin(16).ygap(8);

	layout3.add(layout2);
	layout3.add(layout1);

	win.add(layout3);
}

void wiresalmon::handle_event(rtk::events::menu_selection& ev)
{
	if (ev.target() == &ibhelp) {
		os::Wimp_StartTask("Filer_Run <Wiresalmon$Dir>.!Help", 0);
	} else if (ev.target() == &ibquit) {
		if (stop.enabled()) {
			stop.enabled(false);
			stop_capture();
		}
		parent_application()->terminate();
	}
}

void wiresalmon::handle_event(events::user_drag_box& ev)
{
	os::pointer_info_get info;
	os::Wimp_GetPointerInfo(info);

	os::wimp_block& block = *new os::wimp_block;
	block.word[3] = 0;
	block.word[4] = rtk::swi::Message_DataSave;
	block.word[5] = info.whandle;
	block.word[6] = info.ihandle;
	block.word[7] = info.p.x();
	block.word[8] = info.p.y();
	block.word[9]= 0;
	block.word[10] = 0xfff;
	string::size_type offset = pathname.text().find_last_of(".:");
	if (offset == string::npos) {
		offset = 0;
	} else {
		offset++;
	}
	unsigned i = pathname.text().substr(offset).copy(block.byte+44,211);
	block.byte[44 + i] = 0;
	block.word[0] = 44 + ((i + 4) & ~3);

	send_message(rtk::swi::User_Message, block, info.whandle, info.ihandle);
	add(saveop);
}

void salmonsave::handle_event(events::datasaveack& ev)
{
	wiresalmon &app = *dynamic_cast<wiresalmon *>(parent_application());
	app.pathname.text(ev.pathname());
}

void wiresalmon::handle_event(rtk::events::mouse_click& ev)
{
	if (ev.target() == &ibicon) {
		if (ev.buttons() == 2) {
			ibmenu.show(ev);
		} else if (ev.buttons() == 4) {
			// Find centre of desktop.
			box dbbox(bbox());
			point dcentre((dbbox.xmin()+dbbox.xmax())/2, (dbbox.ymin()+dbbox.ymax())/2);
			add(win, dcentre);
		}
	} else if (ev.target() == &filetype) {
		if (ev.buttons() & 0x40) {
			filetype.drag_sprite(filetype.bbox(), (os::sprite_area*)1, "file_ffd");
		}
	} else if ((ev.target() == &start) && (ev.buttons() == 4) && start.enabled()) {
		if (pathname.text().find_first_of(".:<") == string::npos) {
			throw "To save, drag the icon to a directory display";
		}
		stop.enabled(true);
		start.enabled(false);
		filetype.enabled(false);
		pathname.enabled(false);

		start_capture(pathname.text().c_str());

	} else if ((ev.target() == &stop) && (ev.buttons() == 4) && stop.enabled()) {
		stop.enabled(false);
		start.enabled(true);
		filetype.enabled(true);
		pathname.enabled(true);

		stop_capture();
	}
}

int main(void)
{
	// Check correct module version is loaded. Do this here rather than
	// the !Run file to avoid hardcoding the version number
	int ret = _kernel_oscli("RMEnsure Wiresalmon " Module_VersionString " RMLoad <Wiresalmon$Dir>.Wiresalmon");
	if (ret != -2) ret = _kernel_oscli("RMEnsure Wiresalmon " Module_VersionString " Error xyz");
	if (ret == -2) {
		os::Wimp_ReportError(1, "Wiresalmon " Module_VersionString " module not found", "Wiresalmon", 0, 0);
		return 1;
	}

	wiresalmon app;
	app.run();
	app.kill_module();
	return 0;
}

#include "swis.h"

void wiresalmon::start_capture(const char *filename)
{
	_kernel_oserror *err;

	err = _swix(Wiresalmon_Start, _INR(0, 1), filename, 0);
	if (err) throw err->errmess;
}

void wiresalmon::stop_capture(void)
{
	_kernel_oserror *err;
	err = _swix(Wiresalmon_Stop, 0);
	if (err) throw err->errmess;
}

void wiresalmon::kill_module(void)
{
	_swix(OS_Module, _INR(0,1), 4, "Wiresalmon");
}

