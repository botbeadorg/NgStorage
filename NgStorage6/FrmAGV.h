// ---------------------------------------------------------------------------

#ifndef FrmAGVH
#define FrmAGVH
// ---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Ani.hpp>
#include <FMX.Colors.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Effects.hpp>
#include <FMX.Objects.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Grid.hpp>
#include <FMX.Grid.Style.hpp>
#include <FMX.ScrollBox.hpp>
#include <System.Rtti.hpp>
#include <System.UIConsts.hpp>
#include "common.h"
#include <FMX.Filter.Effects.hpp>
#include <FMX.Menus.hpp>

struct half_storloc {
	TCircle *c;
	size_t sn_flag[4]; // storey, area, r, c
	String txt;
};

// ---------------------------------------------------------------------------
class TFrame1 : public TFrame {
__published: // IDE-managed Components
	TColorButton *ColorButton1;
	TShadowEffect *ShadowEffect1;
	TText *Text1;
	TImage *Image1;
	TShadowEffect *ShadowEffect7;
	TFloatAnimation *FloatAnimation2;
	TLayout *Layout1;
	TGridLayout *GridLayout1;
	TGridPanelLayout *GridPanelLayout1;
	TStringGrid *StringGrid1;
	TScrollBox *ScrollBox1;
	TLayout *Layout2;
	TCircle *Circle1;
	TRectangle *Rectangle1;
	TLabel *Label1;
	TLine *Line1;
	TText *Text2;
	TText *Text3;
	TPopupMenu *PopupMenu1;
	TMenuItem *MenuItem1;

	void __fastcall ColorButton1Click(TObject *Sender);
	void __fastcall FloatAnimation2Finish(TObject *Sender);
	void __fastcall Circle1Click(TObject *Sender);
	void __fastcall Rectangle1Click(TObject *Sender);
	void __fastcall Circle1DblClick(TObject *Sender);
	void __fastcall Circle1MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
		float X, float Y);
	void __fastcall Circle1MouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, float X,
		float Y);
	void __fastcall ScrollBox1DblClick(TObject *Sender);
	void __fastcall ScrollBox1MouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, float X,
          float Y);
	void __fastcall MenuItem1Click(TObject *Sender);
	void __fastcall Text3Click(TObject *Sender);
	void __fastcall Text2Click(TObject *Sender);

private: // User declarations
public: // User declarations
	__fastcall TFrame1(TComponent* Owner);
	__fastcall ~TFrame1();

	float max_w, max_h;
	float scale;
	TStringDynArray agvprms;
	DynamicArray<double>agv_colors;
	DynamicArray<TColorButton*>agv_buttons;
	DynamicArray<TFloatAnimation*>fas;
	DynamicArray<TText*>txts;
	DynamicArray<TColumn*>cols;
	DynamicArray<TLayout*>layouts;
	DynamicArray<TPointF>scales;
	DynamicArray<TMySelection*>slcts;
	DynamicArray<TRectangle*>rects;

	std::vector<storloc*>storlocs;

	matrix1 m;
	matrix M;
	matrix M1;

	TLayout *crt_layout;
	TAniCalculations *anic;

	TLayout *fullview;
	ThreadUpdateAgview *thrd_ua;

	half_storloc half_sl;

	std::vector<agvtask_listitem>at_items;

	CRITICAL_SECTION cs_at_items;

	std::vector<area_storloc>columns_plan;

	DynamicArray<TText*>txt_mycoords;
	DynamicArray<DynamicArray<TText*> >txt_mycoords_2d;

	void __fastcall add_color_buttons();
	void __fastcall layout_zoomin(TLayout *);
	void __fastcall layout_zoomout(TLayout *);
	void __fastcall add_agv_title();
	void __fastcall draw_basis();
	void __fastcall draw_path(double color, loc_base b, loc_base e);
	void __fastcall add_schdl_title();
	void __fastcall add_slct_title();
	void __fastcall stor_loc_display(TObject *Sender);
	TPoint __fastcall circle_center(TCircle *);
	TPoint __fastcall rect_center(TRectangle *);
	void __fastcall agvtask_lineto(agvtask_view *);
	size_t __fastcall control_type(TObject *);
	void __fastcall restore_task_end_point(agvtask_view *);
	void __fastcall wipe_line(agvtask_view *);
	void __fastcall build_column_plan();
};

// ---------------------------------------------------------------------------
extern PACKAGE TFrame1 *Frame1;
// ---------------------------------------------------------------------------
#endif
