/*
	Frontend for browsing and creating mounts


	Copyright (C) 2006 Alex Waugh

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
#include "rtk/swi/wimp.h"


#include "sunfish-frontend.h"


using namespace std;
using namespace rtk;
using namespace rtk::desktop;
using rtk::graphics::point;
using rtk::graphics::box;


ibicon::ibicon(const std::string& icontext, const std::string& special, bool usetcp, int version):
	specialfield(special),
	tcp(usetcp),
	nfsversion(version)
{
	proginfo.add("Name", "Sunfish");
	proginfo.add("Purpose", "Mount NFS servers");
	proginfo.add("Author", "? Alex Waugh, 2003-2010");
	proginfo.add("Version", Module_VersionString " (" Module_Date ")");
	ibinfo.text("Info");
	ibinfo.attach_dbox(proginfo);
	ibhelp.text("Help...");
	ibbrowse.text("Browse...");
	ibsave.text("Save mounts");
	ibdismount.text("Dismount");
	ibfree.text("Free...");
	ibquit.text("Quit");
	ibmenu.title("Sunfish");
	ibmenu.add(ibinfo);
	ibmenu.add(ibhelp);
	ibmenu.add(ibbrowse);
	ibmenu.add(ibsave);
	ibmenu.add(ibdismount);
	ibmenu.add(ibfree);
	ibmenu.add(ibquit);

	if (icontext == "") {
		ibdismount.enabled(false);
		ibfree.enabled(false);
		sprite_name("!Sunfish").hcentre(true);
	} else {
		text(icontext).hcentre(true);
		text_and_sprite(true).validation("Sfile_1b6");
	}
	attach_menu(ibmenu);
	position(-5).priority(0x10000000);
}

string ibicon::filename()
{
	string name = "Sunfish";

	if (specialfield.length() > 0) {
		name += "#";
		name += specialfield;
	}
	name += "::";
	name += text();
	name += ".$";

	return name;
}

void ibicon::opendir()
{
	string cmd = "Filer_OpenDir ";

	cmd += filename();
	os::Wimp_StartTask(cmd.c_str(), 0);
}

void ibicon::closedir()
{
	string cmd = "Filer_CloseDir ";

	cmd += filename();
	os::Wimp_StartTask(cmd.c_str(), 0);
}

void ibicon::handle_event(rtk::events::menu_selection& ev)
{
	sunfish& app = *static_cast<sunfish *>(parent_application());

	if (ev.target() == &ibhelp) {
		os::Wimp_StartTask("Filer_Run <Sunfish$Dir>.!Help", 0);
	} else if (ev.target() == &ibbrowse) {
		app.browserwin.open(app);
	} else if (ev.target() == &ibdismount) {
		closedir();
		dismount();
	} else if (ev.target() == &ibfree) {
		string cmd = "ShowFree -fs Sunfish ";

		if (specialfield.length() > 0) {
			cmd += specialfield;
			cmd += "#";
		}
		cmd += text();
		os::Wimp_StartTask(cmd.c_str(), 0);
	} else if (ev.target() == &ibquit) {
		parent_application()->terminate();
	}
}

void ibicon::handle_event(rtk::events::mouse_click& ev)
{
	sunfish& app = *static_cast<sunfish *>(parent_application());

	if (ev.target() == this) {
		if (ev.buttons() == 2) {
			ibmenu.show(ev);
		} else if ((ev.buttons() == 4) && text_and_sprite()) {
			opendir();
		} else {
			app.browserwin.open(app);
		}
	}
}

void ibicon::handle_event(rtk::events::datasave& ev)
{
	if (text() == "") return;

	string pathname = filename();

	pathname += '.';

	const char *defaultpath = getenv("Sunfish$DefaultPath");

	if (defaultpath) {
		pathname += defaultpath;
		pathname += '.';
	}

	pathname += ev.leafname();

	ev.reply(pathname, true);
}

void ibicon::handle_event(rtk::events::dataload& ev)
{
	if (text() == "") return;

	if (ev.wimpblock().word[3] == 0) {
		// Your ref is 0, so an original message not a reply
		string pathname = filename();

		const char *defaultpath = getenv("Sunfish$DefaultPath");

		if (defaultpath) {
			pathname += '.';
			pathname += defaultpath;
		}

		os::wimp_block block;
		int size = 24 + pathname.length();
		size &= ~3;
		ev.prepare_reply(block, swi::Message_FilerDeviceDir, size);
		snprintf(block.byte + 20, sizeof(block) - 20, "%s", pathname.c_str());
		os::Wimp_SendMessage(swi::User_Message, block, ev.thandle(), 0, 0);

		string cmd = "Filer_OpenDir ";

		cmd += pathname;
		os::Wimp_StartTask(cmd.c_str(), 0);
	} else {
		string cmd = "Filer_OpenDir ";

		cmd += ev.pathname();

		cmd.erase(cmd.rfind("."));

		os::Wimp_StartTask(cmd.c_str(), 0);

		ev.reply();
	}
}

#include <swis.h>

void ibicon::mount(const char *config)
{
	_kernel_oserror *err = _swix(Sunfish_Mount, _INR(0,2), text().c_str(), specialfield.length() ? specialfield.c_str() : NULL, config);
	if (err) throw err->errmess;
}

void ibicon::dismount()
{
	_kernel_oserror *err = _swix(Sunfish_Dismount, _INR(0,1), text().c_str(), specialfield.length() ? specialfield.c_str() : NULL);
	if (err) throw err->errmess;
}
