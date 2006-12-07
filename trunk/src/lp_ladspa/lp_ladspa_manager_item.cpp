#include "lp_ladspa_manager_item.h"

lp_ladspa_manager_item::lp_ladspa_manager_item(QWidget *parent)
	: QWidget(parent)
{
	pv_plugin = 0;
	pv_lister = 0;
	pv_channels = 0;
	pv_buf_size = 0;
	pv_samplerate = 0;

	setupUi(this);

	pb_remove->setText(tr("Remove"));
	pb_select->setText(tr("Select"));
	pb_edit->setText(tr("Edit"));
	pb_activate->setText(tr("Activate"));

	connect(pb_remove, SIGNAL(clicked()), this, SLOT(remove_plugin()));
	connect(pb_select, SIGNAL(clicked()), this, SLOT(select_plugin()));
	connect(pb_edit, SIGNAL(clicked()), this, SLOT(edit_plugin()));
	connect(pb_activate, SIGNAL(clicked()), this, SLOT(activate_plugin()));
}

lp_ladspa_manager_item::~lp_ladspa_manager_item()
{
	if(pv_plugin != 0){
		delete pv_plugin;
	}
}

int lp_ladspa_manager_item::init(int channels, int buf_size, int samplerate)
{
	pv_channels = channels;
	pv_buf_size = buf_size;
	pv_samplerate = samplerate;
	pv_lister = new lp_ladspa_lister;
	connect(pv_lister, SIGNAL(signal_ok()), this, SLOT(init_plugin()));
	return 0;
}

int lp_ladspa_manager_item::run_interlaced_buffer(float *buffer, int tot_len)
{
	if(pv_plugin != 0){
		return pv_plugin->run_interlaced_buffer(buffer, tot_len);
	}
	return 0;
}

void lp_ladspa_manager_item::remove_plugin()
{
	if(pv_plugin != 0){
		delete pv_plugin;
		pv_plugin = 0;
	}
	pb_select->setText(tr("Select"));
	pb_edit->setText(tr("Edit"));
	pb_activate->setText(tr("Activate"));
}

void lp_ladspa_manager_item::select_plugin()
{
	if(pv_lister == 0){
		std::cerr << "lp_ladspa_manager_item::" << __FUNCTION__ << ": pv_lister is Null\n";
		return;
	}
	pv_lister->setWindowTitle(tr("LADSPA plugin selection"));
	pv_lister->show();
}

void lp_ladspa_manager_item::init_plugin()
{
	pv_lister->close();
	if(pv_plugin != 0){
		delete pv_plugin;
		pv_plugin = 0;
	}
	pv_plugin = new lp_ladspa_plugin;
	pv_plugin->init(pv_lister->get_ladspa_descriptor(), pv_channels, pv_buf_size, pv_samplerate);
	pb_select->setText(pv_plugin->get_name());
	pb_edit->setText(tr("Edit"));
	pb_activate->setText(tr("Activate"));
}

void lp_ladspa_manager_item::edit_plugin()
{
	if(pv_plugin != 0){
		if(pv_plugin->isHidden() == true){
			pv_plugin->show();
			pb_edit->setText(tr("In edition"));
		}else{
			pv_plugin->hide();
			pb_edit->setText(tr("Edit"));
		}
	}
}

void lp_ladspa_manager_item::activate_plugin()
{
	if(pv_plugin != 0){
		if(pv_plugin->is_active() != true){
			pv_plugin->activate();
			pb_activate->setText(tr("Active"));
		}else{
			pv_plugin->deactivate();
			pb_activate->setText(tr("Activate"));
		}
	}
}
