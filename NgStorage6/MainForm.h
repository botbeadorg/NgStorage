// ---------------------------------------------------------------------------

#ifndef MainFormH
#define MainFormH
// ---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Menus.hpp>
#include <FMX.Types.hpp>
#include <FMX.Layouts.hpp>
#include <System.IniFiles.hpp>
#include "common.h"
#include "MatrixForm.h"
#include <FMX.Objects.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Colors.hpp>
#include <FMX.Effects.hpp>
#include <FMX.Grid.hpp>
#include <FMX.Grid.Style.hpp>
#include <FMX.ScrollBox.hpp>
#include <System.Rtti.hpp>
#include <FMX.Filter.Effects.hpp>
#include <FMX.Ani.hpp>
#include <FMX.ListBox.hpp>
#include <FMX.Edit.hpp>

enum indx_req_docs {
	ngstorage, req_docOther
};

enum indx_req_params {
	cmd, paramOther
};

// ---------------------------------------------------------------------------
class TForm1 : public TForm {
__published: // IDE-managed Components
	TStyleBook *StyleBook1;
	TToolBar *ToolBar1;
	TCornerButton *CornerButton6;
	TCornerButton *CornerButton7;
	TImage *Image1;
	TImage *Image2;
	TFloatAnimation *FloatAnimation1;
	TFloatAnimation *FloatAnimation2;
	TCornerButton *CornerButton9;
	TComboBox *ComboBox1;
	TEdit *Edit2;
	TEdit *Edit1;
	TSpeedButton *SpeedButton1;
	TCornerButton *CornerButton1;
	TCornerButton *CornerButton2;
	TStatusBar *StatusBar1;
	TLabel *Label1;
	TLabel *Label2;
	TCheckBox *CheckBox1;

	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FloatAnimation2Finish(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall Image1Click(TObject *Sender);
	void __fastcall Image2Click(TObject *Sender);
	void __fastcall FormKeyUp(TObject *Sender, WORD &Key, System::WideChar &KeyChar,
		TShiftState Shift);
	void __fastcall CornerButton9Click(TObject *Sender);
	void __fastcall Edit1Change(TObject *Sender);
	void __fastcall SpeedButton1Click(TObject *Sender);
	void __fastcall ComboBox1Change(TObject *Sender);
	void __fastcall CornerButton2Click(TObject *Sender);
	void __fastcall ColorComboBox1Change(TObject *Sender);
	// void __fastcall ComboBox2Change(TObject *Sender);
	void __fastcall Edit3ChangeTracking(TObject *Sender);

private: // User declarations
public: // User declarations
	__fastcall TForm1(TComponent* Owner);
	__fastcall ~TForm1();
	void __fastcall boot(String root);

	bool thrd_wp_stop;

	SOCKET mysock;
	u_long my_addr_b;
	size_t No_cmd_base, No_cmd_crt, No_cmd_limit;

	String home;
	String conf;
	String spec_package;
	TIntegerSet exes_status;
	TStringDynArray exes_paths;
	SHELLEXECUTEINFO launch_infos[UI];
	agv agvs[LEN_AGVS];

	int id_warehouse;
	size_t count_agvs;
	size_t real_count_agvs;
	agv_n *info_agvs;
	conf_profile cfp;

	DynamicArray<TLayout*>blocks;
	DynamicArray<TMenuItem*>menuitems;
	DynamicArray<TForm3*>storey_forms;
	DynamicArray<TCornerButton*>buttons_storey;
	DynamicArray<TLabel*>agv_labels;
	TStringDynArray popup_params;
	TStringDynArray agvprms;
	DynamicArray<double>agv_colors;
	DynamicArray<TColorButton*>agv_buttons;
	DynamicArray<TFloatAnimation*>fas;
	DynamicArray<bool>hittest_b_storey;

	TCornerButton *b_schdl;
	TCornerButton *b_schdl_confirm;

	matrix m;

	std::vector<agvtask_view*>q_agvt;

	agvtask_view av_pending;

	CRITICAL_SECTION cs_agviews;

	std::queue<waypoint*>wps;

	CRITICAL_SECTION cs_wps;
	CONDITION_VARIABLE cv_wps;
	uintptr_t thrd_wp;

	uintptr_t thrd_rcv;

	HANDLE draw_basis_end;

	std::vector<task_issue>tis;

	CRITICAL_SECTION cs_tis;

	records_spec_pack records_sp;

	coords *flags_coords;

	void __fastcall conf_template();
	void __fastcall parse_config();

	void __fastcall build_matrix();
	void __fastcall draw_plan2();
	void __fastcall add_storey_buttons();
	void __fastcall storey_click(TObject *);
	void __fastcall add_color_buttons();

	void __fastcall slct_addition();
	void __fastcall confirm_schdl_click(TObject *);
	void __fastcall schdl_click(TObject *);
	void __fastcall btn_fsm(size_t);
	void __fastcall btn_context_init();

	void __fastcall client_conn();
	bool __fastcall idle_agv(size_t *);
	bool __fastcall get_agvtask_type(size_t *);
	void __fastcall issue_task(agvtask_view *);
	void __fastcall issue_task_http(TDateTime *, agvtask_view *);
	size_t __fastcall number_cmd();
	bool __fastcall allow_issue();
	void __fastcall init_agvtask_view(agvtask_view *);
	void __fastcall spec_package_template();
	void __fastcall parse_spec_package();
};

class TThrdProcRefCircleColor : public TCppInterfacedObject<Classes::TThreadProcedure> {
private:
	size_t colr;
	TCircle *circ;

public:
	TThrdProcRefCircleColor(TCircle *control, size_t color) : circ(control), colr(color) {
	}

	void __fastcall Invoke() {
		circ->Fill->Color = colr;
	}
};

class TThrdProcRefRectColor : public TCppInterfacedObject<Classes::TThreadProcedure> {
private:
	size_t colr;
	TRectangle *rect;

public:
	TThrdProcRefRectColor(TRectangle *control, size_t color) : rect(control), colr(color) {
	}

	void __fastcall Invoke() {
		rect->Fill->Color = colr;
	}
};

class TThrdProcRefEditText : public TCppInterfacedObject<Classes::TThreadProcedure> {
private:
	String txt;
	TEdit *edit;

public:
	TThrdProcRefEditText(TEdit *control, String str) : edit(control), txt(str) {
	}

	void __fastcall Invoke() {
		edit->Text = txt;
	}
};

class TThrdProcRefEditTagChage : public TCppInterfacedObject<Classes::TThreadProcedure> {
private:
	int tagvalue;

	System::TObject* tagobj;

	TEdit *edit;

public:
	TThrdProcRefEditTagChage(TEdit *control, int tagv, System::TObject *tago)
		: edit(control), tagvalue(tagv), tagobj(tago) {
	}

	void __fastcall Invoke() {
		edit->Tag = tagvalue;
		edit->TagObject = tagobj;
	}
};

class TThrdProcRefAddLine : public TCppInterfacedObject<Classes::TThreadProcedure> {
private:
	TLayout *layout;
	TLine *line;

public:
	TThrdProcRefAddLine(TLayout *lyt, TLine *ln) : layout(lyt), line(ln) {
	}

	void __fastcall Invoke() {
		layout->AddObject(line);
	}
};


class TThrdProcRefLabelText : public TCppInterfacedObject<Classes::TThreadProcedure> {
private:
	TLabel *label;
	String txt;

public:
	TThrdProcRefLabelText(TLabel *lbl, String t) : label(lbl), txt(t) {
	}

	void __fastcall Invoke() {
        label->Text = txt;
	}
};

class TThrdProcRefRemoveLine : public TCppInterfacedObject<Classes::TThreadProcedure> {
private:
	TLayout *layout;
	TLine *line;

public:
	TThrdProcRefRemoveLine(TLayout *lyt, TLine *ln) : layout(lyt), line(ln) {
	}

	void __fastcall Invoke() {
        line->Visible = 0;
		layout->RemoveObject(line);
        line->DisposeOf();
	}
};

// ---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
// ---------------------------------------------------------------------------
#endif
