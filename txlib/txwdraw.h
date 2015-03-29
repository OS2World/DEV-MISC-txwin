#ifndef TXWDRAW_H
#define TXWDRAW_H
//
//                     TxWin, Textmode Windowing Library
//
//   Original code Copyright (c) 1995-2005 Fsys Software and Jan van Wijk
//
// ==========================================================================
//
// This file contains Original Code and/or Modifications of Original Code as
// defined in and that are subject to the GNU Lesser General Public License.
// You may not use this file except in compliance with the License.
// BY USING THIS FILE YOU AGREE TO ALL TERMS AND CONDITIONS OF THE LICENSE.
// A copy of the License is provided with the Original Code and Modifications,
// and is also available at http://www.dfsee.com/txwin/lgpl.htm
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation; either version 2.1 of the License,
// or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; (lgpl.htm) if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// Questions on TxWin licensing can be directed to: txwin@fsys.nl
//
// ==========================================================================
//
//
// TX Windowed text, drawing, color-scheme definitions
//
// Author: J. van Wijk
//
// JvW  16-04-2004 Implement color scheme data structures

//- define the actual color-mappings, to be used in the colorschemes below

//- New default scheme, a 3D-look with Grey menus, Cyan borders and White Windows
static TXW_COLORMAP    txwcm_3g =
{
   0,                                           // cSchemeColor
   TX_Black_on_White,                           // cWinClientClr
   TX_Black_on_White,                           // cWinClientNr0
   TX_Blue_on_White,                            // cWinClientNr1
   TX_Green_on_White,                           // cWinClientNr2
   TX_Cyan_on_White,                            // cWinClientNr3
   TX_Red_on_White,                             // cWinClientNr4
   TX_Magenta_on_White,                         // cWinClientNr5
   TX_Yellow_on_White,                          // cWinClientNr6
   TX_Grey_on_White,                            // cWinClientNr7
   TX_Lwhite_on_White,                          // cWinBorder_top
   TX_Lwhite_on_White,                          // cWinBorder_trc
   TX_Black_on_White,                           // cWinBorder_rgt
   TX_Black_on_White,                           // cWinBorder_brc
   TX_Black_on_White,                           // cWinBorder_bot
   TX_Lwhite_on_White,                          // cWinBorder_blc
   TX_Lwhite_on_White,                          // cWinBorder_lft
   TX_Lwhite_on_White,                          // cWinBorder_tlc
   TX_Lwhite_on_White,                          // cWinTitleStand
   TX_Lwhite_on_Grey,                           // cWinTitleFocus
   TX_Black_on_White,                           // cWinFooterStand
   TX_Lwhite_on_Grey,                           // cWinFooterFocus
   TX_Lwhite_on_White,                          // cDlgBorder_top
   TX_Black_on_White,                           // cDlgBorder_trc
   TX_Black_on_White,                           // cDlgBorder_rgt
   TX_Black_on_White,                           // cDlgBorder_brc
   TX_Black_on_White,                           // cDlgBorder_bot
   TX_Lwhite_on_White,                          // cDlgBorder_blc
   TX_Lwhite_on_White,                          // cDlgBorder_lft
   TX_Lwhite_on_White,                          // cDlgBorder_tlc
   TX_Black_on_White,                           // cDlgTitleStand
   TX_Black_on_White,                           // cDlgTitleFocus
   TX_Black_on_White,                           // cDlgFooterStand
   TX_Black_on_White,                           // cDlgFooterFocus
   TX_White_on_Grey,                            // cPushBorder_top
   TX_White_on_Grey,                            // cPushBorder_trc
   TX_Black_on_Grey,                            // cPushBorder_rgt
   TX_Black_on_Grey,                            // cPushBorder_brc
   TX_Black_on_Grey,                            // cPushBorder_bot
   TX_White_on_Grey,                            // cPushBorder_blc
   TX_White_on_Grey,                            // cPushBorder_lft
   TX_White_on_Grey,                            // cPushBorder_tlc
   TX_Yellow_on_Cyan,                           // cPushTitleStand
   TX_Lwhite_on_Magenta,                        // cPushTitleFocus
   TX_Cyan_on_Cyan,                             // cPushFooterStand
   TX_Yellow_on_Blue,                           // cPushFooterFocus
   TX_Lwhite_on_White,                          // cViewBorder_top
   TX_Lwhite_on_White,                          // cViewBorder_trc
   TX_Black_on_White,                           // cViewBorder_rgt
   TX_Black_on_White,                           // cViewBorder_brc
   TX_Black_on_White,                           // cViewBorder_bot
   TX_Lwhite_on_White,                          // cViewBorder_blc
   TX_Lwhite_on_White,                          // cViewBorder_lft
   TX_Lwhite_on_White,                          // cViewBorder_tlc
   TX_Lwhite_on_White,                          // cViewTitleStand
   TX_Lwhite_on_Grey,                           // cViewTitleFocus
   TX_Black_on_White,                           // cViewFooterStand
   TX_White_on_Grey,                            // cViewFooterFocus
   TX_Black_on_Cyan,                            // cSbvBorder_top
   TX_Lcyan_on_Cyan,                            // cSbvBorder_trc (or blank)
   TX_Lcyan_on_Cyan,                            // cSbvBorder_rgt
   TX_Lcyan_on_Cyan,                            // cSbvBorder_brc
   TX_Lcyan_on_Cyan,                            // cSbvBorder_bot
   TX_Black_on_Cyan,                            // cSbvBorder_blc (or blank)
   TX_Black_on_Cyan,                            // cSbvBorder_lft
   TX_Black_on_Cyan,                            // cSbvBorder_tlc
   TX_Yellow_on_Cyan,                           // cSbvTitleStand
   TX_Lwhite_on_Magenta,                        // cSbvTitleFocus
   TX_Lcyan_on_Cyan,                            // cSbvFooterStand
   TX_Lcyan_on_Cyan,                            // cSbvFooterFocus
   TX_Yellow_on_Cyan,                           // cSbvStatusStand
   TX_Yellow_on_Cyan,                           // cSbvStatusFocus
   TX_Lgreen_on_Cyan,                           // cSbvProgreStand
   TX_Lgreen_on_Cyan,                           // cSbvTraceStand
   TX_White_on_Magenta,                         // cSbvMarkedArea
   TX_Lgreen_on_Cyan,                           // cDskTraceStand
   TX_Lcyan_on_Cyan,                            // cDskBorder_top
   TX_Lcyan_on_Cyan,                            // cDskBorder_trc (or blank)
   TX_Black_on_Cyan,                            // cDskBorder_rgt
   TX_Black_on_Cyan,                            // cDskBorder_brc
   TX_Black_on_Cyan,                            // cDskBorder_bot
   TX_Lcyan_on_Cyan,                            // cDskBorder_blc (or blank)
   TX_Lcyan_on_Cyan,                            // cDskBorder_lft
   TX_Lcyan_on_Cyan,                            // cDskBorder_tlc
   TX_Lwhite_on_Cyan,                           // cDskTitleStand
   TX_Lwhite_on_Cyan,                           // cDskTitleFocus
   TX_White_on_Blue,                            // cDskFooterStand
   TX_White_on_Blue,                            // cDskFooterFocus
   TX_Black_on_White,                           // cLineTextStand
   TX_Black_on_White,                           // cLineTextNr0
   TX_Blue_on_White,                            // cLineTextNr1
   TX_Green_on_White,                           // cLineTextNr2
   TX_Cyan_on_White,                            // cLineTextNr3
   TX_Red_on_White,                             // cLineTextNr4
   TX_Magenta_on_White,                         // cLineTextNr5
   TX_Yellow_on_White,                          // cLineTextNr6
   TX_Grey_on_White,                            // cLineTextNr7
   TX_Black_on_White,                           // cHelpTextStand
   TX_Black_on_White,                           // cHelpTextHelp
   TX_Black_on_White,                           // cViewTextStand
   TX_White_on_Magenta,                         // cViewTextMark
   TX_Blue_on_White,                            // cViewTextNr1
   TX_Green_on_White,                           // cViewTextNr2
   TX_Cyan_on_White,                            // cViewTextNr3
   TX_Red_on_White,                             // cViewTextNr4
   TX_Magenta_on_White,                         // cViewTextNr5
   TX_Yellow_on_White,                          // cViewTextNr6
   TX_Grey_on_White,                            // cViewTextNr7
   TX_Black_on_White,                           // cListTextStand
   TX_Lgreen_on_Black,                          // cListTextFocus
   TX_Green_on_White,                           // cListSelectStand
   TX_Lgreen_on_Black,                          // cListSelectFocus
   TX_Magenta_on_White,                         // cListMarkStand
   TX_Magenta_on_Black,                         // cListMarkFocus
   TX_Lwhite_on_White,                          // cListDisableStand
   TX_White_on_Blue,                            // cListDisableFocus
   TX_Black_on_White,                           // cListSeparatStand
   TX_Green_on_Black,                           // cListSeparatFocus
   TX_White_on_Cyan,                            // cListCountStand
   TX_Yellow_on_Cyan,                           // cListCountFocus
   TX_Black_on_White,                           // cFileTextStand
   TX_Yellow_on_Black,                          // cFileTextFocus
   TX_Black_on_White,                           // cFileSelectStand
   TX_Yellow_on_Black,                          // cFileSelectFocus
   TX_Magenta_on_White,                         // cFileMarkStand
   TX_Magenta_on_Black,                         // cFileMarkFocus
   TX_Lwhite_on_White,                          // cFileDisableStand
   TX_White_on_Blue,                            // cFileDisableFocus
   TX_Black_on_White,                           // cFileSeparatStand
   TX_Yellow_on_Black,                          // cFileSeparatFocus
   TX_Black_on_White,                           // cFileCountStand
   TX_Blue_on_White,                            // cFileCountFocus
   TX_Black_on_White,                           // cFileBorder_top
   TX_Lwhite_on_White,                          // cFileBorder_trc (or blank)
   TX_Lwhite_on_White,                          // cFileBorder_rgt
   TX_Lwhite_on_White,                          // cFileBorder_brc
   TX_Lwhite_on_White,                          // cFileBorder_bot
   TX_Lwhite_on_White,                          // cFileBorder_blc (or blank)
   TX_Black_on_White,                           // cFileBorder_lft
   TX_Black_on_White,                           // cFileBorder_tlc
   TX_Black_on_White,                           // cFileTitleStand
   TX_Lwhite_on_Grey,                           // cFileTitleFocus
   TX_Lwhite_on_White,                          // cFileFooterStand
   TX_White_on_Grey,                            // cFileFooterFocus
   TX_Black_on_White,                           // cApplTextStand
   TX_Lcyan_on_Black,                           // cApplTextFocus
   TX_Cyan_on_White,                            // cApplSelectStand
   TX_Lgreen_on_Black,                          // cApplSelectFocus
   TX_Magenta_on_White,                         // cApplMarkStand
   TX_Magenta_on_Black,                         // cApplMarkFocus
   TX_Lwhite_on_White,                          // cApplDisableStand
   TX_White_on_Blue,                            // cApplDisableFocus
   TX_Black_on_White,                           // cApplSeparatStand
   TX_Green_on_Black,                           // cApplSeparatFocus
   TX_White_on_Grey,                            // cMenuTextStand
   TX_White_on_Blue,                            // cMenuTextFocus
   TX_Yellow_on_Grey,                           // cMenuSelectStand
   TX_Yellow_on_Blue,                           // cMenuSelectFocus
   TX_Black_on_Grey,                            // cMenuMarkStand
   TX_Cyan_on_Blue,                             // cMenuMarkFocus            (mbar)
   TX_Black_on_Grey,                            // cMenuDisableStand
   TX_Cyan_on_Blue,                             // cMenuDisableFocus         (mbar)
   TX_White_on_Grey,                            // cMenuSeparatStand
   TX_White_on_Blue,                            // cMenuSeparatFocus
   TX_White_on_Grey,                            // cMenuBorder_top
   TX_White_on_Grey,                            // cMenuBorder_trc
   TX_Black_on_Grey,                            // cMenuBorder_rgt
   TX_Black_on_Grey,                            // cMenuBorder_brc
   TX_Black_on_Grey,                            // cMenuBorder_bot
   TX_White_on_Grey,                            // cMenuBorder_blc
   TX_White_on_Grey,                            // cMenuBorder_lft
   TX_White_on_Grey,                            // cMenuBorder_tlc
   TX_White_on_Grey,                            // cMbarBorder_top
   TX_White_on_Grey,                            // cMbarBorder_trc
   TX_White_on_Grey,                            // cMbarBorder_rgt
   TX_White_on_Grey,                            // cMbarBorder_brc
   TX_White_on_Grey,                            // cMbarBorder_bot
   TX_White_on_Grey,                            // cMbarBorder_blc
   TX_White_on_Grey,                            // cMbarBorder_lft
   TX_White_on_Grey,                            // cMbarBorder_tlc
   TX_White_on_Grey,                            // cMbarTextStand
   TX_White_on_Grey,                            // cMbarTextFocus
   TX_Yellow_on_Grey,                           // cMbarHeadStand
   TX_Lwhite_on_Magenta,                        // cMbarHeadFocus
   TX_White_on_Grey,                            // cSpinTextStand
   TX_Lwhite_on_Grey,                           // cSpinTextFocus
   TX_Yellow_on_Grey,                           // cSpinSelectStand
   TX_Yellow_on_Grey,                           // cSpinSelectFocus
   TX_Black_on_Grey,                            // cSpinMarkStand
   TX_Black_on_Grey,                            // cSpinMarkFocus
   TX_Black_on_Grey,                            // cSpinDisableStand
   TX_Black_on_Grey,                            // cSpinDisableFocus
   TX_White_on_Grey,                            // cSpinSeparatStand
   TX_White_on_Grey,                            // cSpinSeparatFocus
   TX_Lwhite_on_Grey,                           // cSpinIndcStand
   TX_White_on_Blue,                            // cSpinIndcFocus
   TX_Lwhite_on_Grey,                           // cPushTextStand
   TX_Yellow_on_Grey,                           // cPushTextFocus
   TX_Black_on_White,                           // cRadioValueStand
   TX_Black_on_White,                           // cRadioValueFocus
   TX_Black_on_White,                           // cRadioTextStand
   TX_Black_on_White,                           // cRadioTextFocus
   TX_Black_on_White,                           // cCheckValueStand
   TX_Black_on_White,                           // cCheckValueFocus
   TX_Black_on_White,                           // cCheckTextStand
   TX_Black_on_White,                           // cCheckTextFocus
   TX_Yellow_on_Green,                          // cDlgEfTextStand
   TX_Lwhite_on_Green,                          // cDlgEfTextFocus
   TX_Yellow_on_Green,                          // cEntryTextStand
   TX_Yellow_on_Green,                          // cEntryTextFocus
   TX_Green_on_Green,                           // cEntryHistStand
   TX_White_on_Green,                           // cEntryHistFocus
   TX_White_on_Green,                           // cEntrBorder_top
   TX_White_on_Green,                           // cEntrBorder_trc
   TX_White_on_Green,                           // cEntrBorder_rgt
   TX_White_on_Green,                           // cEntrBorder_brc
   TX_White_on_Green,                           // cEntrBorder_bot
   TX_White_on_Green,                           // cEntrBorder_blc
   TX_White_on_Green,                           // cEntrBorder_lft
   TX_White_on_Green,                           // cEntrBorder_tlc
   TX_White_on_Green,                           // cEntrTitleStand
   TX_Lwhite_on_Grey,                           // cEntrTitleFocus
   TX_Black_on_White,                           // cEntrFooterStand
   TX_White_on_Grey,                            // cEntrFooterFocus
   TX_Black_on_White,                           // cMLEntTextStand
   TX_Black_on_White,                           // cMLEntTextFocus
   TX_Yellow_on_Magenta,                        // cHexEdCurByteChg
   TX_Lwhite_on_Magenta,                        // cHexEdCursorByte
   TX_White_on_Blue,                            // cHexEdHexByteStd
   TX_Yellow_on_Blue,                           // cHexEdHexByteChg
   TX_White_on_Magenta,                         // cHexEdHexByteMrk
   TX_Yellow_on_Magenta,                        // cHexEdHexByteMch
   TX_Lwhite_on_Blue,                           // cHexEdAscByteStd
   TX_Lcyan_on_Blue,                            // cHexEdAscBracket
   TX_Grey_on_White,                            // cHexEdRelPosPrev
   TX_Black_on_White,                           // cHexEdRelPosCurr
   TX_Grey_on_White,                            // cHexEdRelPosNext
   TX_Cyan_on_White,                            // cHexEdAbsBytePos
   TX_Cyan_on_White,                            // cHexEdAbsByteCur
   TX_Black_on_White,                           // cHexEdRelCursorP
   TX_Yellow_on_Grey,                           // cHexEdButtonText
   TX_White_on_Grey,                            // cHexEdButBracket
   TX_Grey_on_White,                            // cHexEdByteNumber
   TX_White_on_Cyan,                            // cHexEdItemSnText
   TX_Yellow_on_Cyan,                           // cHexEdItemHlight
   TX_Red_on_White,                             // cHexEdModifyText
};

//- Color scheme that avoids the 'bright backgrounds' that may cause blinking
static TXW_COLORMAP    txwcm_nb =
{
   0,                                           // cSchemeColor
   TX_Black_on_White,                           // cWinClientClr
   TX_Black_on_White,                           // cWinClientNr0
   TX_Blue_on_White,                            // cWinClientNr1
   TX_Green_on_White,                           // cWinClientNr2
   TX_Cyan_on_White,                            // cWinClientNr3
   TX_Red_on_White,                             // cWinClientNr4
   TX_Magenta_on_White,                         // cWinClientNr5
   TX_Yellow_on_White,                          // cWinClientNr6
   TX_Grey_on_White,                            // cWinClientNr7
   TX_White_on_Cyan,                            // cWinBorder_top
   TX_White_on_Cyan,                            // cWinBorder_trc
   TX_White_on_Cyan,                            // cWinBorder_rgt
   TX_White_on_Cyan,                            // cWinBorder_brc
   TX_White_on_Cyan,                            // cWinBorder_bot
   TX_White_on_Cyan,                            // cWinBorder_blc
   TX_White_on_Cyan,                            // cWinBorder_lft
   TX_White_on_Cyan,                            // cWinBorder_tlc
   TX_Yellow_on_Cyan,                           // cWinTitleStand
   TX_Lwhite_on_Magenta,                        // cWinTitleFocus
   TX_Cyan_on_Cyan,                             // cWinFooterStand
   TX_Yellow_on_Blue,                           // cWinFooterFocus
   TX_Black_on_White,                           // cDlgBorder_top
   TX_Black_on_White,                           // cDlgBorder_trc
   TX_Black_on_White,                           // cDlgBorder_rgt
   TX_Black_on_White,                           // cDlgBorder_brc
   TX_Black_on_White,                           // cDlgBorder_bot
   TX_Black_on_White,                           // cDlgBorder_blc
   TX_Black_on_White,                           // cDlgBorder_lft
   TX_Black_on_White,                           // cDlgBorder_tlc
   TX_Black_on_White,                           // cDlgTitleStand
   TX_Black_on_White,                           // cDlgTitleFocus
   TX_Black_on_White,                           // cDlgFooterStand
   TX_Black_on_White,                           // cDlgFooterFocus
   TX_Lwhite_on_Cyan,                           // cPushBorder_top
   TX_Lwhite_on_Cyan,                           // cPushBorder_trc
   TX_Lwhite_on_Cyan,                           // cPushBorder_rgt
   TX_Lwhite_on_Cyan,                           // cPushBorder_brc
   TX_Lwhite_on_Cyan,                           // cPushBorder_bot
   TX_Lwhite_on_Cyan,                           // cPushBorder_blc
   TX_Lwhite_on_Cyan,                           // cPushBorder_lft
   TX_Lwhite_on_Cyan,                           // cPushBorder_tlc
   TX_Yellow_on_Cyan,                           // cPushTitleStand
   TX_Lwhite_on_Magenta,                        // cPushTitleFocus
   TX_Cyan_on_Cyan,                             // cPushFooterStand
   TX_Yellow_on_Blue,                           // cPushFooterFocus
   TX_White_on_Cyan,                            // cViewBorder_top
   TX_White_on_Cyan,                            // cViewBorder_trc
   TX_White_on_Cyan,                            // cViewBorder_rgt
   TX_White_on_Cyan,                            // cViewBorder_brc
   TX_White_on_Cyan,                            // cViewBorder_bot
   TX_White_on_Cyan,                            // cViewBorder_blc
   TX_White_on_Cyan,                            // cViewBorder_lft
   TX_White_on_Cyan,                            // cViewBorder_tlc
   TX_Yellow_on_Cyan,                           // cViewTitleStand
   TX_Lwhite_on_Magenta,                        // cViewTitleFocus
   TX_Cyan_on_Cyan,                             // cViewFooterStand
   TX_Yellow_on_Blue,                           // cViewFooterFocus
   TX_White_on_Cyan,                            // cSbvBorder_top
   TX_White_on_Cyan,                            // cSbvBorder_trc
   TX_White_on_Cyan,                            // cSbvBorder_rgt
   TX_White_on_Cyan,                            // cSbvBorder_brc
   TX_White_on_Cyan,                            // cSbvBorder_bot
   TX_White_on_Cyan,                            // cSbvBorder_blc
   TX_White_on_Cyan,                            // cSbvBorder_lft
   TX_White_on_Cyan,                            // cSbvBorder_tlc
   TX_Yellow_on_Cyan,                           // cSbvTitleStand
   TX_Lwhite_on_Magenta,                        // cSbvTitleFocus
   TX_White_on_Cyan,                            // cSbvFooterStand
   TX_White_on_Cyan,                            // cSbvFooterFocus
   TX_Yellow_on_Cyan,                           // cSbvStatusStand
   TX_Yellow_on_Cyan,                           // cSbvStatusFocus
   TX_Lgreen_on_Cyan,                           // cSbvProgreStand
   TX_Lgreen_on_Cyan,                           // cSbvTraceStand
   TX_White_on_Magenta,                         // cSbvMarkedArea
   TX_Lgreen_on_Cyan,                           // cDskTraceStand
   TX_White_on_Cyan,                            // cDskBorder_top
   TX_White_on_Cyan,                            // cDskBorder_trc
   TX_White_on_Cyan,                            // cDskBorder_rgt
   TX_White_on_Cyan,                            // cDskBorder_brc
   TX_White_on_Cyan,                            // cDskBorder_bot
   TX_White_on_Cyan,                            // cDskBorder_blc
   TX_White_on_Cyan,                            // cDskBorder_lft
   TX_White_on_Cyan,                            // cDskBorder_tlc
   TX_Lwhite_on_Cyan,                           // cDskTitleStand
   TX_Lwhite_on_Cyan,                           // cDskTitleFocus
   TX_White_on_Blue,                            // cDskFooterStand
   TX_White_on_Blue,                            // cDskFooterFocus
   TX_Black_on_White,                           // cLineTextStand
   TX_Black_on_White,                           // cLineTextNr0
   TX_Blue_on_White,                            // cLineTextNr1
   TX_Green_on_White,                           // cLineTextNr2
   TX_Cyan_on_White,                            // cLineTextNr3
   TX_Red_on_White,                             // cLineTextNr4
   TX_Magenta_on_White,                         // cLineTextNr5
   TX_Yellow_on_White,                          // cLineTextNr6
   TX_Grey_on_White,                            // cLineTextNr7
   TX_Black_on_White,                           // cHelpTextStand
   TX_Black_on_White,                           // cHelpTextHelp
   TX_Black_on_White,                           // cViewTextStand
   TX_White_on_Magenta,                         // cViewTextMark
   TX_Blue_on_White,                            // cViewTextNr1
   TX_Green_on_White,                           // cViewTextNr2
   TX_Cyan_on_White,                            // cViewTextNr3
   TX_Red_on_White,                             // cViewTextNr4
   TX_Magenta_on_White,                         // cViewTextNr5
   TX_Yellow_on_White,                          // cViewTextNr6
   TX_Grey_on_White,                            // cViewTextNr7
   TX_White_on_Brown,                           // cListTextStand
   TX_White_on_Blue,                            // cListTextFocus
   TX_Yellow_on_Brown,                          // cListSelectStand
   TX_Yellow_on_Blue,                           // cListSelectFocus
   TX_Black_on_Brown,                           // cListMarkStand
   TX_Cyan_on_Blue,                             // cListMarkFocus            (mbar)
   TX_Black_on_Brown,                           // cListDisableStand
   TX_Cyan_on_Blue,                             // cListDisableFocus         (mbar)
   TX_White_on_Brown,                           // cListSeparatStand
   TX_White_on_Blue,                            // cListSeparatFocus
   TX_White_on_Cyan,                            // cListCountStand
   TX_Yellow_on_Cyan,                           // cListCountFocus
   TX_Black_on_White,                           // cFileTextStand
   TX_Green_on_Black,                           // cFileTextFocus
   TX_Black_on_White,                           // cFileSelectStand
   TX_Green_on_Black,                           // cFileSelectFocus
   TX_Magenta_on_White,                         // cFileMarkStand
   TX_Magenta_on_Black,                         // cFileMarkFocus
   TX_Grey_on_Black,                            // cFileDisableStand
   TX_Cyan_on_Blue,                             // cFileDisableFocus         (mbar)
   TX_Black_on_White,                           // cFileSeparatStand
   TX_Green_on_Black,                           // cFileSeparatFocus
   TX_White_on_Cyan,                            // cFileCountStand
   TX_Yellow_on_Cyan,                           // cFileCountFocus
   TX_White_on_Cyan,                            // cFileBorder_top
   TX_White_on_Cyan,                            // cFileBorder_trc
   TX_White_on_Cyan,                            // cFileBorder_rgt
   TX_White_on_Cyan,                            // cFileBorder_brc
   TX_White_on_Cyan,                            // cFileBorder_bot
   TX_White_on_Cyan,                            // cFileBorder_blc
   TX_White_on_Cyan,                            // cFileBorder_lft
   TX_White_on_Cyan,                            // cFileBorder_tlc
   TX_Yellow_on_Cyan,                           // cFileTitleStand
   TX_Lwhite_on_Magenta,                        // cFileTitleFocus
   TX_Cyan_on_Cyan,                             // cFileFooterStand
   TX_Yellow_on_Blue,                           // cFileFooterFocus
   TX_Black_on_White,                           // cApplTextStand
   TX_Green_on_Black,                           // cApplTextFocus
   TX_Black_on_White,                           // cApplSelectStand
   TX_Green_on_Black,                           // cApplSelectFocus
   TX_Magenta_on_White,                         // cApplMarkStand
   TX_Magenta_on_Black,                         // cApplMarkFocus
   TX_Grey_on_Black,                            // cApplDisableStand
   TX_Cyan_on_Blue,                             // cApplDisableFocus         (mbar)
   TX_Black_on_White,                           // cApplSeparatStand
   TX_Green_on_Black,                           // cApplSeparatFocus
   TX_White_on_Brown,                           // cMenuTextStand
   TX_White_on_Blue,                            // cMenuTextFocus
   TX_Yellow_on_Brown,                          // cMenuSelectStand
   TX_Yellow_on_Blue,                           // cMenuSelectFocus
   TX_Black_on_Brown,                           // cMenuMarkStand
   TX_Cyan_on_Blue,                             // cMenuMarkFocus            (mbar)
   TX_Black_on_Brown,                           // cMenuDisableStand
   TX_Cyan_on_Blue,                             // cMenuDisableFocus         (mbar)
   TX_White_on_Brown,                           // cMenuSeparatStand
   TX_White_on_Blue,                            // cMenuSeparatFocus
   TX_White_on_Cyan,                            // cMenuBorder_top
   TX_White_on_Cyan,                            // cMenuBorder_trc
   TX_White_on_Cyan,                            // cMenuBorder_rgt
   TX_White_on_Cyan,                            // cMenuBorder_brc
   TX_White_on_Cyan,                            // cMenuBorder_bot
   TX_White_on_Cyan,                            // cMenuBorder_blc
   TX_White_on_Cyan,                            // cMenuBorder_lft
   TX_White_on_Cyan,                            // cMenuBorder_tlc
   TX_White_on_Brown,                           // cMbarBorder_top
   TX_White_on_Brown,                           // cMbarBorder_trc
   TX_White_on_Brown,                           // cMbarBorder_rgt
   TX_White_on_Brown,                           // cMbarBorder_brc
   TX_White_on_Brown,                           // cMbarBorder_bot
   TX_White_on_Brown,                           // cMbarBorder_blc
   TX_White_on_Brown,                           // cMbarBorder_lft
   TX_White_on_Brown,                           // cMbarBorder_tlc
   TX_White_on_Brown,                           // cMbarTextStand
   TX_White_on_Brown,                           // cMbarTextFocus
   TX_Yellow_on_Brown,                          // cMbarHeadStand
   TX_Lwhite_on_Magenta,                        // cMbarHeadFocus
   TX_White_on_Brown,                           // cSpinTextStand
   TX_Lwhite_on_Brown,                          // cSpinTextFocus
   TX_Yellow_on_Brown,                          // cSpinSelectStand
   TX_Yellow_on_Brown,                          // cSpinSelectFocus
   TX_Black_on_Brown,                           // cSpinMarkStand
   TX_Black_on_Brown,                           // cSpinMarkFocus
   TX_Black_on_Brown,                           // cSpinDisableStand
   TX_Black_on_Brown,                           // cSpinDisableFocus
   TX_White_on_Brown,                           // cSpinSeparatStand
   TX_White_on_Brown,                           // cSpinSeparatFocus
   TX_Brown_on_Brown,                           // cSpinIndcStand
   TX_White_on_Blue,                            // cSpinIndcFocus
   TX_Lwhite_on_Cyan,                           // cPushTextStand
   TX_Yellow_on_Cyan,                           // cPushTextFocus
   TX_Black_on_White,                           // cRadioValueStand
   TX_Black_on_White,                           // cRadioValueFocus
   TX_Black_on_White,                           // cRadioTextStand
   TX_Black_on_White,                           // cRadioTextFocus
   TX_Black_on_White,                           // cCheckValueStand
   TX_Black_on_White,                           // cCheckValueFocus
   TX_Black_on_White,                           // cCheckTextStand
   TX_Black_on_White,                           // cCheckTextFocus
   TX_Yellow_on_Green,                          // cDlgEfTextStand
   TX_Lwhite_on_Green,                          // cDlgEfTextFocus
   TX_Yellow_on_Green,                          // cEntryTextStand
   TX_Yellow_on_Green,                          // cEntryTextFocus
   TX_Green_on_Green,                           // cEntryHistStand
   TX_White_on_Green,                           // cEntryHistFocus
   TX_White_on_Green,                           // cEntrBorder_top
   TX_White_on_Green,                           // cEntrBorder_trc
   TX_White_on_Green,                           // cEntrBorder_rgt
   TX_White_on_Green,                           // cEntrBorder_brc
   TX_White_on_Green,                           // cEntrBorder_bot
   TX_White_on_Green,                           // cEntrBorder_blc
   TX_White_on_Green,                           // cEntrBorder_lft
   TX_White_on_Green,                           // cEntrBorder_tlc
   TX_White_on_Green,                           // cEntrTitleStand
   TX_Lwhite_on_Magenta,                        // cEntrTitleFocus
   TX_White_on_Green,                           // cEntrFooterStand
   TX_White_on_Blue,                            // cEntrFooterFocus
   TX_Black_on_White,                           // cMLEntTextStand
   TX_Black_on_White,                           // cMLEntTextFocus
   TX_Yellow_on_Magenta,                        // cHexEdCurByteChg
   TX_Lwhite_on_Magenta,                        // cHexEdCursorByte
   TX_White_on_Blue,                            // cHexEdHexByteStd
   TX_Yellow_on_Blue,                           // cHexEdHexByteChg
   TX_White_on_Magenta,                         // cHexEdHexByteMrk
   TX_Yellow_on_Magenta,                        // cHexEdHexByteMch
   TX_White_on_Blue,                            // cHexEdAscByteStd
   TX_Lcyan_on_Blue,                            // cHexEdAscBracket
   TX_Grey_on_White,                            // cHexEdRelPosPrev
   TX_Black_on_White,                           // cHexEdRelPosCurr
   TX_Grey_on_White,                            // cHexEdRelPosNext
   TX_Cyan_on_White,                            // cHexEdAbsBytePos
   TX_Cyan_on_White,                            // cHexEdAbsByteCur
   TX_Black_on_White,                           // cHexEdRelCursorP
   TX_Yellow_on_Cyan,                           // cHexEdButtonText
   TX_White_on_Cyan,                            // cHexEdButBracket
   TX_Grey_on_White,                            // cHexEdByteNumber
   TX_White_on_Magenta,                         // cHexEdItemSnText
   TX_Yellow_on_Magenta,                        // cHexEdItemHlight
   TX_Red_on_White,                             // cHexEdModifyText
};

//- Color scheme inspired by the Norton Commander and clones, very Cyan/Blue
static TXW_COLORMAP    txwcm_cm =
{
   0,                                           // cSchemeColor
   TX_Black_on_White,                           // cWinClientClr
   TX_Black_on_White,                           // cWinClientNr0
   TX_Blue_on_White,                            // cWinClientNr1
   TX_Green_on_White,                           // cWinClientNr2
   TX_Cyan_on_White,                            // cWinClientNr3
   TX_Red_on_White,                             // cWinClientNr4
   TX_Magenta_on_White,                         // cWinClientNr5
   TX_Yellow_on_White,                          // cWinClientNr6
   TX_Grey_on_White,                            // cWinClientNr7
   TX_Black_on_White,                           // cWinBorder_top
   TX_Black_on_White,                           // cWinBorder_trc
   TX_Black_on_White,                           // cWinBorder_rgt
   TX_Black_on_White,                           // cWinBorder_brc
   TX_Black_on_White,                           // cWinBorder_bot
   TX_Black_on_White,                           // cWinBorder_blc
   TX_Black_on_White,                           // cWinBorder_lft
   TX_Black_on_White,                           // cWinBorder_tlc
   TX_Black_on_White,                           // cWinTitleStand
   TX_Blue_on_White,                            // cWinTitleFocus
   TX_Black_on_White,                           // cWinFooterStand
   TX_Magenta_on_White,                         // cWinFooterFocus
   TX_Black_on_White,                           // cDlgBorder_top
   TX_Black_on_White,                           // cDlgBorder_trc
   TX_Black_on_White,                           // cDlgBorder_rgt
   TX_Black_on_White,                           // cDlgBorder_brc
   TX_Black_on_White,                           // cDlgBorder_bot
   TX_Black_on_White,                           // cDlgBorder_blc
   TX_Black_on_White,                           // cDlgBorder_lft
   TX_Black_on_White,                           // cDlgBorder_tlc
   TX_Black_on_White,                           // cDlgTitleStand
   TX_Black_on_White,                           // cDlgTitleFocus
   TX_Black_on_White,                           // cDlgFooterStand
   TX_Black_on_White,                           // cDlgFooterFocus
   TX_Black_on_White,                           // cPushBorder_top
   TX_Black_on_White,                           // cPushBorder_trc
   TX_Black_on_White,                           // cPushBorder_rgt
   TX_Black_on_White,                           // cPushBorder_brc
   TX_Black_on_White,                           // cPushBorder_bot
   TX_Black_on_White,                           // cPushBorder_blc
   TX_Black_on_White,                           // cPushBorder_lft
   TX_Black_on_White,                           // cPushBorder_tlc
   TX_Black_on_White,                           // cPushTitleStand
   TX_Black_on_Cyan,                            // cPushTitleFocus
   TX_Black_on_White,                           // cPushFooterStand
   TX_Black_on_Cyan,                            // cPushFooterFocus
   TX_Black_on_White,                           // cViewBorder_top
   TX_Black_on_White,                           // cViewBorder_trc
   TX_Black_on_White,                           // cViewBorder_rgt
   TX_Black_on_White,                           // cViewBorder_brc
   TX_Black_on_White,                           // cViewBorder_bot
   TX_Black_on_White,                           // cViewBorder_blc
   TX_Black_on_White,                           // cViewBorder_lft
   TX_Black_on_White,                           // cViewBorder_tlc
   TX_Black_on_White,                           // cViewTitleStand
   TX_Blue_on_White,                            // cViewTitleFocus
   TX_Black_on_White,                           // cViewFooterStand
   TX_Magenta_on_White,                         // cViewFooterFocus
   TX_Lcyan_on_Blue,                            // cSbvBorder_top
   TX_Lcyan_on_Blue,                            // cSbvBorder_trc
   TX_Lcyan_on_Blue,                            // cSbvBorder_rgt
   TX_Lcyan_on_Blue,                            // cSbvBorder_brc
   TX_Lcyan_on_Blue,                            // cSbvBorder_bot
   TX_Lcyan_on_Blue,                            // cSbvBorder_blc
   TX_Lcyan_on_Blue,                            // cSbvBorder_lft
   TX_Lcyan_on_Blue,                            // cSbvBorder_tlc
   TX_Lcyan_on_Blue,                            // cSbvTitleStand
   TX_Black_on_Cyan,                            // cSbvTitleFocus
   TX_Lcyan_on_Blue,                            // cSbvFooterStand
   TX_Lcyan_on_Blue,                            // cSbvFooterFocus
   TX_Yellow_on_Blue,                           // cSbvStatusStand
   TX_Yellow_on_Blue,                           // cSbvStatusFocus
   TX_Lgreen_on_Blue,                           // cSbvProgreStand
   TX_Lgreen_on_Blue,                           // cSbvTraceStand
   TX_White_on_Magenta,                         // cSbvMarkedArea
   TX_Lgreen_on_Black,                          // cDskTraceStand
   TX_White_on_Black,                           // cDskBorder_top
   TX_White_on_Black,                           // cDskBorder_trc
   TX_White_on_Black,                           // cDskBorder_rgt
   TX_White_on_Black,                           // cDskBorder_brc
   TX_White_on_Black,                           // cDskBorder_bot
   TX_White_on_Black,                           // cDskBorder_blc
   TX_White_on_Black,                           // cDskBorder_lft
   TX_White_on_Black,                           // cDskBorder_tlc
   TX_Lwhite_on_Black,                          // cDskTitleStand
   TX_Lwhite_on_Black,                          // cDskTitleFocus
   TX_Lgreen_on_Black,                          // cDskFooterStand
   TX_Lgreen_on_Black,                          // cDskFooterFocus
   TX_Black_on_White,                           // cLineTextStand
   TX_Black_on_White,                           // cLineTextNr0
   TX_Blue_on_White,                            // cLineTextNr1
   TX_Green_on_White,                           // cLineTextNr2
   TX_Cyan_on_White,                            // cLineTextNr3
   TX_Red_on_White,                             // cLineTextNr4
   TX_Magenta_on_White,                         // cLineTextNr5
   TX_Yellow_on_White,                          // cLineTextNr6
   TX_Grey_on_White,                            // cLineTextNr7
   TX_Black_on_White,                           // cHelpTextStand
   TX_Black_on_White,                           // cHelpTextHelp
   TX_Black_on_White,                           // cViewTextStand
   TX_White_on_Magenta,                         // cViewTextMark
   TX_Blue_on_White,                            // cViewTextNr1
   TX_Green_on_White,                           // cViewTextNr2
   TX_Cyan_on_White,                            // cViewTextNr3
   TX_Red_on_White,                             // cViewTextNr4
   TX_Magenta_on_White,                         // cViewTextNr5
   TX_Yellow_on_White,                          // cViewTextNr6
   TX_Grey_on_White,                            // cViewTextNr7
   TX_Lwhite_on_Cyan,                           // cListTextStand
   TX_Lwhite_on_Black,                          // cListTextFocus
   TX_Yellow_on_Cyan,                           // cListSelectStand
   TX_Yellow_on_Black,                          // cListSelectFocus
   TX_Lwhite_on_Cyan,                           // cListMarkStand
   TX_Lwhite_on_Black,                          // cListMarkFocus
   TX_White_on_Cyan,                            // cListDisableStand
   TX_White_on_Black,                           // cListDisableFocus
   TX_Black_on_Cyan,                            // cListSeparatStand
   TX_White_on_Black,                           // cListSeparatFocus
   TX_White_on_Cyan,                            // cListCountStand
   TX_Yellow_on_Cyan,                           // cListCountFocus
   TX_Lgreen_on_Blue,                           // cFileTextStand
   TX_Black_on_Green,                           // cFileTextFocus
   TX_Lgreen_on_Blue,                           // cFileSelectStand
   TX_Black_on_Green,                           // cFileSelectFocus
   TX_Yellow_on_Blue,                           // cFileMarkStand
   TX_Yellow_on_Green,                          // cFileMarkFocus
   TX_Grey_on_Black,                            // cFileDisableStand
   TX_Cyan_on_Blue,                             // cFileDisableFocus         (mbar)
   TX_Lgreen_on_Blue,                           // cFileSeparatStand
   TX_Black_on_Green,                           // cFileSeparatFocus
   TX_Lcyan_on_Blue,                            // cFileCountStand
   TX_Yellow_on_Blue,                           // cFileCountFocus
   TX_Lcyan_on_Blue,                            // cFileBorder_top
   TX_Lcyan_on_Blue,                            // cFileBorder_trc
   TX_Lcyan_on_Blue,                            // cFileBorder_rgt
   TX_Lcyan_on_Blue,                            // cFileBorder_brc
   TX_Lcyan_on_Blue,                            // cFileBorder_bot
   TX_Lcyan_on_Blue,                            // cFileBorder_blc
   TX_Lcyan_on_Blue,                            // cFileBorder_lft
   TX_Lcyan_on_Blue,                            // cFileBorder_tlc
   TX_Lcyan_on_Blue,                            // cFileTitleStand
   TX_Black_on_Cyan,                            // cFileTitleFocus
   TX_Lcyan_on_Blue,                            // cFileFooterStand
   TX_Black_on_Cyan,                            // cFileFooterFocus
   TX_Black_on_White,                           // cApplTextStand
   TX_Green_on_Black,                           // cApplTextFocus
   TX_Black_on_White,                           // cApplSelectStand
   TX_Green_on_Black,                           // cApplSelectFocus
   TX_Magenta_on_White,                         // cApplMarkStand
   TX_Magenta_on_Black,                         // cApplMarkFocus
   TX_Grey_on_Black,                            // cApplDisableStand
   TX_Cyan_on_Blue,                             // cApplDisableFocus         (mbar)
   TX_Black_on_White,                           // cApplSeparatStand
   TX_Green_on_Black,                           // cApplSeparatFocus
   TX_Lwhite_on_Cyan,                           // cMenuTextStand
   TX_Lwhite_on_Black,                          // cMenuTextFocus
   TX_Yellow_on_Cyan,                           // cMenuSelectStand
   TX_Yellow_on_Black,                          // cMenuSelectFocus
   TX_Lwhite_on_Cyan,                           // cMenuMarkStand
   TX_Lwhite_on_Black,                          // cMenuMarkFocus
   TX_White_on_Cyan,                            // cMenuDisableStand
   TX_White_on_Black,                           // cMenuDisableFocus
   TX_Black_on_Cyan,                            // cMenuSeparatStand
   TX_White_on_Black,                           // cMenuSeparatFocus
   TX_Black_on_Cyan,                            // cMenuBorder_top
   TX_Black_on_Cyan,                            // cMenuBorder_trc
   TX_Black_on_Cyan,                            // cMenuBorder_rgt
   TX_Black_on_Cyan,                            // cMenuBorder_brc
   TX_Black_on_Cyan,                            // cMenuBorder_bot
   TX_Black_on_Cyan,                            // cMenuBorder_blc
   TX_Black_on_Cyan,                            // cMenuBorder_lft
   TX_Black_on_Cyan,                            // cMenuBorder_tlc
   TX_Black_on_Cyan,                            // cMbarBorder_top
   TX_Black_on_Cyan,                            // cMbarBorder_trc
   TX_Black_on_Cyan,                            // cMbarBorder_rgt
   TX_Black_on_Cyan,                            // cMbarBorder_brc
   TX_Black_on_Cyan,                            // cMbarBorder_bot
   TX_Black_on_Cyan,                            // cMbarBorder_blc
   TX_Black_on_Cyan,                            // cMbarBorder_lft
   TX_Black_on_Cyan,                            // cMbarBorder_tlc
   TX_Black_on_Cyan,                            // cMbarTextStand
   TX_Black_on_Cyan,                            // cMbarTextFocus
   TX_Black_on_Cyan,                            // cMbarHeadStand
   TX_Lwhite_on_Black,                          // cMbarHeadFocus
   TX_Black_on_Cyan,                            // cSpinTextStand
   TX_Lwhite_on_Cyan,                           // cSpinTextFocus
   TX_Black_on_Cyan,                            // cSpinSelectStand
   TX_Yellow_on_Cyan,                           // cSpinSelectFocus
   TX_Black_on_Cyan,                            // cSpinMarkStand
   TX_Black_on_Cyan,                            // cSpinMarkFocus
   TX_Black_on_Cyan,                            // cSpinDisableStand
   TX_Black_on_Cyan,                            // cSpinDisableFocus
   TX_Black_on_Cyan,                            // cSpinSeparatStand
   TX_Black_on_Cyan,                            // cSpinSeparatFocus
   TX_Cyan_on_Cyan,                             // cSpinIndcStand
   TX_Lcyan_on_Blue,                            // cSpinIndcFocus
   TX_Black_on_White,                           // cPushTextStand
   TX_Black_on_Cyan,                            // cPushTextFocus
   TX_Black_on_White,                           // cRadioValueStand
   TX_Blue_on_White,                            // cRadioValueFocus
   TX_Black_on_White,                           // cRadioTextStand
   TX_Black_on_White,                           // cRadioTextFocus
   TX_Black_on_White,                           // cCheckValueStand
   TX_Blue_on_White,                            // cCheckValueFocus
   TX_Black_on_White,                           // cCheckTextStand
   TX_Black_on_White,                           // cCheckTextFocus
   TX_Black_on_Cyan,                            // cDlgEfTextStand
   TX_Lwhite_on_Cyan,                           // cDlgEfTextFocus
   TX_White_on_Black,                           // cEntryTextStand
   TX_White_on_Black,                           // cEntryTextFocus
   TX_Black_on_Black,                           // cEntryHistStand
   TX_White_on_Black,                           // cEntryHistFocus
   TX_White_on_Black,                           // cEntrBorder_top
   TX_White_on_Black,                           // cEntrBorder_trc
   TX_White_on_Black,                           // cEntrBorder_rgt
   TX_White_on_Black,                           // cEntrBorder_brc
   TX_White_on_Black,                           // cEntrBorder_bot
   TX_White_on_Black,                           // cEntrBorder_blc
   TX_White_on_Black,                           // cEntrBorder_lft
   TX_White_on_Black,                           // cEntrBorder_tlc
   TX_Lcyan_on_Blue,                            // cEntrTitleStand
   TX_Black_on_Cyan,                            // cEntrTitleFocus
   TX_Lcyan_on_Blue,                            // cEntrFooterStand
   TX_Black_on_Cyan,                            // cEntrFooterFocus
   TX_Black_on_White,                           // cMLEntTextStand
   TX_Black_on_White,                           // cMLEntTextFocus
   TX_Yellow_on_Magenta,                        // cHexEdCurByteChg
   TX_Lwhite_on_Magenta,                        // cHexEdCursorByte
   TX_Lcyan_on_Blue,                            // cHexEdHexByteStd
   TX_Yellow_on_Blue,                           // cHexEdHexByteChg
   TX_White_on_Magenta,                         // cHexEdHexByteMrk
   TX_Yellow_on_Magenta,                        // cHexEdHexByteMch
   TX_Lgreen_on_Blue,                           // cHexEdAscByteStd
   TX_Lcyan_on_Blue,                            // cHexEdAscBracket
   TX_Cyan_on_Blue,                             // cHexEdRelPosPrev
   TX_Lcyan_on_Blue,                            // cHexEdRelPosCurr
   TX_Cyan_on_Blue,                             // cHexEdRelPosNext
   TX_Green_on_Blue,                            // cHexEdAbsBytePos
   TX_Green_on_White,                           // cHexEdAbsByteCur
   TX_Cyan_on_White,                            // cHexEdRelCursorP
   TX_Black_on_Cyan,                            // cHexEdButtonText
   TX_White_on_Cyan,                            // cHexEdButBracket
   TX_Black_on_White,                           // cHexEdByteNumber
   TX_White_on_Magenta,                         // cHexEdItemSnText
   TX_Yellow_on_Magenta,                        // cHexEdItemHlight
   TX_Red_on_White,                             // cHexEdModifyText
};

//- Commander scheme with a 3D-look in Cyan/Black menus and Blue/White Windows
static TXW_COLORMAP    txwcm_3c =
{
   0,                                           // cSchemeColor
   TX_Black_on_White,                           // cWinClientClr
   TX_Black_on_White,                           // cWinClientNr0
   TX_Blue_on_White,                            // cWinClientNr1
   TX_Green_on_White,                           // cWinClientNr2
   TX_Cyan_on_White,                            // cWinClientNr3
   TX_Red_on_White,                             // cWinClientNr4
   TX_Magenta_on_White,                         // cWinClientNr5
   TX_Yellow_on_White,                          // cWinClientNr6
   TX_Grey_on_White,                            // cWinClientNr7
   TX_Cyan_on_White,                            // cWinBorder_top
   TX_Cyan_on_White,                            // cWinBorder_trc
   TX_Black_on_White,                           // cWinBorder_rgt
   TX_Black_on_White,                           // cWinBorder_brc
   TX_Black_on_White,                           // cWinBorder_bot
   TX_Cyan_on_White,                            // cWinBorder_blc
   TX_Cyan_on_White,                            // cWinBorder_lft
   TX_Cyan_on_White,                            // cWinBorder_tlc
   TX_Black_on_White,                           // cWinTitleStand
   TX_Blue_on_White,                            // cWinTitleFocus
   TX_Black_on_White,                           // cWinFooterStand
   TX_Magenta_on_White,                         // cWinFooterFocus
   TX_Cyan_on_White,                            // cDlgBorder_top
   TX_Cyan_on_White,                            // cDlgBorder_trc
   TX_Black_on_White,                           // cDlgBorder_rgt
   TX_Black_on_White,                           // cDlgBorder_brc
   TX_Black_on_White,                           // cDlgBorder_bot
   TX_Cyan_on_White,                            // cDlgBorder_blc
   TX_Cyan_on_White,                            // cDlgBorder_lft
   TX_Cyan_on_White,                            // cDlgBorder_tlc
   TX_Black_on_White,                           // cDlgTitleStand
   TX_Black_on_White,                           // cDlgTitleFocus
   TX_Black_on_White,                           // cDlgFooterStand
   TX_Black_on_White,                           // cDlgFooterFocus
   TX_White_on_Cyan,                            // cPushBorder_top
   TX_White_on_Cyan,                            // cPushBorder_trc
   TX_Black_on_Cyan,                            // cPushBorder_rgt
   TX_Black_on_Cyan,                            // cPushBorder_brc
   TX_Black_on_Cyan,                            // cPushBorder_bot
   TX_White_on_Cyan,                            // cPushBorder_blc
   TX_White_on_Cyan,                            // cPushBorder_lft
   TX_White_on_Cyan,                            // cPushBorder_tlc
   TX_Black_on_Cyan,                            // cPushTitleStand
   TX_Lgreen_on_Cyan,                           // cPushTitleFocus
   TX_Black_on_White,                           // cPushFooterStand
   TX_Black_on_Cyan,                            // cPushFooterFocus
   TX_Cyan_on_White,                            // cViewBorder_top
   TX_Cyan_on_White,                            // cViewBorder_trc
   TX_Black_on_White,                           // cViewBorder_rgt
   TX_Black_on_White,                           // cViewBorder_brc
   TX_Black_on_White,                           // cViewBorder_bot
   TX_Cyan_on_White,                            // cViewBorder_blc
   TX_Cyan_on_White,                            // cViewBorder_lft
   TX_Cyan_on_White,                            // cViewBorder_tlc
   TX_Black_on_White,                           // cViewTitleStand
   TX_Blue_on_White,                            // cViewTitleFocus
   TX_Black_on_White,                           // cViewFooterStand
   TX_Magenta_on_White,                         // cViewFooterFocus
   TX_Cyan_on_Blue,                             // cSbvBorder_top
   TX_Cyan_on_Blue,                             // cSbvBorder_trc (or blank)
   TX_Lcyan_on_Blue,                            // cSbvBorder_rgt
   TX_Lcyan_on_Blue,                            // cSbvBorder_brc
   TX_Lcyan_on_Blue,                            // cSbvBorder_bot
   TX_Cyan_on_Blue,                             // cSbvBorder_blc (or blank)
   TX_Cyan_on_Blue,                             // cSbvBorder_lft
   TX_Cyan_on_Blue,                             // cSbvBorder_tlc
   TX_Lcyan_on_Blue,                            // cSbvTitleStand
   TX_Black_on_Cyan,                            // cSbvTitleFocus
   TX_Lcyan_on_Blue,                            // cSbvFooterStand
   TX_Lcyan_on_Blue,                            // cSbvFooterFocus
   TX_Yellow_on_Blue,                           // cSbvStatusStand
   TX_Yellow_on_Blue,                           // cSbvStatusFocus
   TX_Lgreen_on_Blue,                           // cSbvProgreStand
   TX_Lgreen_on_Blue,                           // cSbvTraceStand
   TX_White_on_Magenta,                         // cSbvMarkedArea
   TX_Lgreen_on_Black,                          // cDskTraceStand
   TX_Lcyan_on_Blue,                            // cDskBorder_top
   TX_Lcyan_on_Blue,                            // cDskBorder_trc (or blank)
   TX_Cyan_on_Blue,                             // cDskBorder_rgt
   TX_Cyan_on_Blue,                             // cDskBorder_brc
   TX_Cyan_on_Blue,                             // cDskBorder_bot
   TX_Lcyan_on_Blue,                            // cDskBorder_blc (or blank)
   TX_Lcyan_on_Blue,                            // cDskBorder_lft
   TX_Lcyan_on_Blue,                            // cDskBorder_tlc
   TX_Lwhite_on_Blue,                           // cDskTitleStand
   TX_Lwhite_on_Blue,                           // cDskTitleFocus
   TX_Black_on_Cyan,                            // cDskFooterStand
   TX_Black_on_Cyan,                            // cDskFooterFocus
   TX_Black_on_White,                           // cLineTextStand
   TX_Black_on_White,                           // cLineTextNr0
   TX_Blue_on_White,                            // cLineTextNr1
   TX_Green_on_White,                           // cLineTextNr2
   TX_Cyan_on_White,                            // cLineTextNr3
   TX_Red_on_White,                             // cLineTextNr4
   TX_Magenta_on_White,                         // cLineTextNr5
   TX_Yellow_on_White,                          // cLineTextNr6
   TX_Grey_on_White,                            // cLineTextNr7
   TX_Black_on_White,                           // cHelpTextStand
   TX_Black_on_White,                           // cHelpTextHelp
   TX_Black_on_White,                           // cViewTextStand
   TX_White_on_Magenta,                         // cViewTextMark
   TX_Blue_on_White,                            // cViewTextNr1
   TX_Green_on_White,                           // cViewTextNr2
   TX_Cyan_on_White,                            // cViewTextNr3
   TX_Red_on_White,                             // cViewTextNr4
   TX_Magenta_on_White,                         // cViewTextNr5
   TX_Yellow_on_White,                          // cViewTextNr6
   TX_Grey_on_White,                            // cViewTextNr7
   TX_Lwhite_on_Cyan,                           // cListTextStand
   TX_Lwhite_on_Black,                          // cListTextFocus
   TX_Red_on_Cyan,                              // cListSelectStand
   TX_White_on_Black,                           // cListSelectFocus
   TX_Lwhite_on_Cyan,                           // cListMarkStand
   TX_Lwhite_on_Black,                          // cListMarkFocus
   TX_White_on_Cyan,                            // cListDisableStand
   TX_White_on_Black,                           // cListDisableFocus
   TX_Black_on_Cyan,                            // cListSeparatStand
   TX_White_on_Black,                           // cListSeparatFocus
   TX_White_on_Cyan,                            // cListCountStand
   TX_Magenta_on_Cyan,                          // cListCountFocus
   TX_Lgreen_on_Blue,                           // cFileTextStand
   TX_Black_on_Green,                           // cFileTextFocus
   TX_Lgreen_on_Blue,                           // cFileSelectStand
   TX_Black_on_Green,                           // cFileSelectFocus
   TX_Yellow_on_Blue,                           // cFileMarkStand
   TX_Yellow_on_Green,                          // cFileMarkFocus
   TX_Grey_on_Black,                            // cFileDisableStand
   TX_Cyan_on_Blue,                             // cFileDisableFocus         (mbar)
   TX_Lgreen_on_Blue,                           // cFileSeparatStand
   TX_Black_on_Green,                           // cFileSeparatFocus
   TX_Lcyan_on_Blue,                            // cFileCountStand
   TX_Yellow_on_Blue,                           // cFileCountFocus
   TX_Lcyan_on_Blue,                            // cFileBorder_top
   TX_Blue_on_Blue,                             // cFileBorder_trc
   TX_Cyan_on_Blue,                             // cFileBorder_rgt
   TX_Cyan_on_Blue,                             // cFileBorder_brc
   TX_Cyan_on_Blue,                             // cFileBorder_bot
   TX_Blue_on_Blue,                             // cFileBorder_blc
   TX_Lcyan_on_Blue,                            // cFileBorder_lft
   TX_Lcyan_on_Blue,                            // cFileBorder_tlc
   TX_Black_on_Cyan,                            // cFileTitleStand
   TX_Lwhite_on_Cyan,                           // cFileTitleFocus
   TX_Black_on_Cyan,                            // cFileFooterStand
   TX_White_on_Cyan,                            // cFileFooterFocus
   TX_Black_on_White,                           // cApplTextStand
   TX_Green_on_Black,                           // cApplTextFocus
   TX_Black_on_White,                           // cApplSelectStand
   TX_Green_on_Black,                           // cApplSelectFocus
   TX_Magenta_on_White,                         // cApplMarkStand
   TX_Magenta_on_Black,                         // cApplMarkFocus
   TX_Grey_on_Black,                            // cApplDisableStand
   TX_Cyan_on_Blue,                             // cApplDisableFocus         (mbar)
   TX_Black_on_White,                           // cApplSeparatStand
   TX_Green_on_Black,                           // cApplSeparatFocus
   TX_Lwhite_on_Cyan,                           // cMenuTextStand
   TX_Lwhite_on_Black,                          // cMenuTextFocus
   TX_Red_on_Cyan,                              // cMenuSelectStand
   TX_White_on_Black,                           // cMenuSelectFocus
   TX_Lwhite_on_Cyan,                           // cMenuMarkStand
   TX_Lwhite_on_Black,                          // cMenuMarkFocus
   TX_White_on_Cyan,                            // cMenuDisableStand
   TX_White_on_Black,                           // cMenuDisableFocus
   TX_Black_on_Cyan,                            // cMenuSeparatStand
   TX_White_on_Black,                           // cMenuSeparatFocus
   TX_White_on_Cyan,                            // cMenuBorder_top
   TX_White_on_Cyan,                            // cMenuBorder_trc
   TX_Black_on_Cyan,                            // cMenuBorder_rgt
   TX_Black_on_Cyan,                            // cMenuBorder_brc
   TX_Black_on_Cyan,                            // cMenuBorder_bot
   TX_White_on_Cyan,                            // cMenuBorder_blc
   TX_White_on_Cyan,                            // cMenuBorder_lft
   TX_White_on_Cyan,                            // cMenuBorder_tlc
   TX_Black_on_Cyan,                            // cMbarBorder_top
   TX_Black_on_Cyan,                            // cMbarBorder_trc
   TX_Black_on_Cyan,                            // cMbarBorder_rgt
   TX_Black_on_Cyan,                            // cMbarBorder_brc
   TX_Black_on_Cyan,                            // cMbarBorder_bot
   TX_Black_on_Cyan,                            // cMbarBorder_blc
   TX_Black_on_Cyan,                            // cMbarBorder_lft
   TX_Black_on_Cyan,                            // cMbarBorder_tlc
   TX_Black_on_Cyan,                            // cMbarTextStand
   TX_Black_on_Cyan,                            // cMbarTextFocus
   TX_Black_on_Cyan,                            // cMbarHeadStand
   TX_Lwhite_on_Black,                          // cMbarHeadFocus
   TX_Black_on_Cyan,                            // cSpinTextStand
   TX_Lwhite_on_Cyan,                           // cSpinTextFocus
   TX_Red_on_Cyan,                              // cSpinSelectStand
   TX_White_on_Black,                           // cSpinSelectFocus
   TX_Black_on_Cyan,                            // cSpinMarkStand
   TX_Black_on_Cyan,                            // cSpinMarkFocus
   TX_Black_on_Cyan,                            // cSpinDisableStand
   TX_Black_on_Cyan,                            // cSpinDisableFocus
   TX_Black_on_Cyan,                            // cSpinSeparatStand
   TX_Black_on_Cyan,                            // cSpinSeparatFocus
   TX_Cyan_on_Cyan,                             // cSpinIndcStand
   TX_Lcyan_on_Blue,                            // cSpinIndcFocus
   TX_Black_on_Cyan,                            // cPushTextStand
   TX_Lgreen_on_Cyan,                           // cPushTextFocus
   TX_Black_on_White,                           // cRadioValueStand
   TX_Blue_on_White,                            // cRadioValueFocus
   TX_Black_on_White,                           // cRadioTextStand
   TX_Black_on_White,                           // cRadioTextFocus
   TX_Black_on_White,                           // cCheckValueStand
   TX_Blue_on_White,                            // cCheckValueFocus
   TX_Black_on_White,                           // cCheckTextStand
   TX_Black_on_White,                           // cCheckTextFocus
   TX_Black_on_Cyan,                            // cDlgEfTextStand
   TX_Lwhite_on_Cyan,                           // cDlgEfTextFocus
   TX_White_on_Black,                           // cEntryTextStand
   TX_White_on_Black,                           // cEntryTextFocus
   TX_Black_on_Black,                           // cEntryHistStand
   TX_White_on_Black,                           // cEntryHistFocus
   TX_White_on_Black,                           // cEntrBorder_top
   TX_White_on_Black,                           // cEntrBorder_trc
   TX_White_on_Black,                           // cEntrBorder_rgt
   TX_White_on_Black,                           // cEntrBorder_brc
   TX_White_on_Black,                           // cEntrBorder_bot
   TX_White_on_Black,                           // cEntrBorder_blc
   TX_White_on_Black,                           // cEntrBorder_lft
   TX_White_on_Black,                           // cEntrBorder_tlc
   TX_Lcyan_on_Blue,                            // cEntrTitleStand
   TX_Black_on_Cyan,                            // cEntrTitleFocus
   TX_Lcyan_on_Blue,                            // cEntrFooterStand
   TX_Black_on_Cyan,                            // cEntrFooterFocus
   TX_Black_on_White,                           // cMLEntTextStand
   TX_Black_on_White,                           // cMLEntTextFocus
   TX_Yellow_on_Magenta,                        // cHexEdCurByteChg
   TX_Lwhite_on_Magenta,                        // cHexEdCursorByte
   TX_Lcyan_on_Blue,                            // cHexEdHexByteStd
   TX_Yellow_on_Blue,                           // cHexEdHexByteChg
   TX_White_on_Magenta,                         // cHexEdHexByteMrk
   TX_Yellow_on_Magenta,                        // cHexEdHexByteMch
   TX_Lgreen_on_Blue,                           // cHexEdAscByteStd
   TX_Lcyan_on_Blue,                            // cHexEdAscBracket
   TX_Brown_on_Blue,                            // cHexEdRelPosPrev
   TX_Lgreen_on_Blue,                           // cHexEdRelPosCurr
   TX_Cyan_on_Blue,                             // cHexEdRelPosNext
   TX_Green_on_Blue,                            // cHexEdAbsBytePos
   TX_Green_on_White,                           // cHexEdAbsByteCur
   TX_Green_on_White,                           // cHexEdRelCursorP
   TX_Black_on_Cyan,                            // cHexEdButtonText
   TX_White_on_Cyan,                            // cHexEdButBracket
   TX_Black_on_White,                           // cHexEdByteNumber
   TX_White_on_Magenta,                         // cHexEdItemSnText
   TX_Lwhite_on_Magenta,                        // cHexEdItemHlight
   TX_Red_on_White,                             // cHexEdModifyText
};

//- Classic color scheme, very close to the original fixed scheme used by DFSee
static TXW_COLORMAP    txwcm_st =
{
   0,                                           // cSchemeColor
   TX_Black_on_White,                           // cWinClientClr
   TX_Black_on_White,                           // cWinClientNr0
   TX_Blue_on_White,                            // cWinClientNr1
   TX_Green_on_White,                           // cWinClientNr2
   TX_Cyan_on_White,                            // cWinClientNr3
   TX_Red_on_White,                             // cWinClientNr4
   TX_Magenta_on_White,                         // cWinClientNr5
   TX_Yellow_on_White,                          // cWinClientNr6
   TX_Grey_on_White,                            // cWinClientNr7
   TX_White_on_Cyan,                            // cWinBorder_top
   TX_White_on_Cyan,                            // cWinBorder_trc
   TX_White_on_Cyan,                            // cWinBorder_rgt
   TX_White_on_Cyan,                            // cWinBorder_brc
   TX_White_on_Cyan,                            // cWinBorder_bot
   TX_White_on_Cyan,                            // cWinBorder_blc
   TX_White_on_Cyan,                            // cWinBorder_lft
   TX_White_on_Cyan,                            // cWinBorder_tlc
   TX_Yellow_on_Cyan,                           // cWinTitleStand
   TX_Lwhite_on_Magenta,                        // cWinTitleFocus
   TX_Cyan_on_Cyan,                             // cWinFooterStand
   TX_Yellow_on_Blue,                           // cWinFooterFocus
   TX_Black_on_White,                           // cDlgBorder_top
   TX_Black_on_White,                           // cDlgBorder_trc
   TX_Black_on_White,                           // cDlgBorder_rgt
   TX_Black_on_White,                           // cDlgBorder_brc
   TX_Black_on_White,                           // cDlgBorder_bot
   TX_Black_on_White,                           // cDlgBorder_blc
   TX_Black_on_White,                           // cDlgBorder_lft
   TX_Black_on_White,                           // cDlgBorder_tlc
   TX_Black_on_White,                           // cDlgTitleStand
   TX_Black_on_White,                           // cDlgTitleFocus
   TX_Black_on_White,                           // cDlgFooterStand
   TX_Black_on_White,                           // cDlgFooterFocus
   TX_Yellow_on_Grey,                           // cPushBorder_top
   TX_Yellow_on_Grey,                           // cPushBorder_trc
   TX_Yellow_on_Grey,                           // cPushBorder_rgt
   TX_Yellow_on_Grey,                           // cPushBorder_brc
   TX_Yellow_on_Grey,                           // cPushBorder_bot
   TX_Yellow_on_Grey,                           // cPushBorder_blc
   TX_Yellow_on_Grey,                           // cPushBorder_lft
   TX_Yellow_on_Grey,                           // cPushBorder_tlc
   TX_Yellow_on_Cyan,                           // cPushTitleStand
   TX_Lwhite_on_Magenta,                        // cPushTitleFocus
   TX_Cyan_on_Cyan,                             // cPushFooterStand
   TX_Yellow_on_Blue,                           // cPushFooterFocus
   TX_White_on_Cyan,                            // cViewBorder_top
   TX_White_on_Cyan,                            // cViewBorder_trc
   TX_White_on_Cyan,                            // cViewBorder_rgt
   TX_White_on_Cyan,                            // cViewBorder_brc
   TX_White_on_Cyan,                            // cViewBorder_bot
   TX_White_on_Cyan,                            // cViewBorder_blc
   TX_White_on_Cyan,                            // cViewBorder_lft
   TX_White_on_Cyan,                            // cViewBorder_tlc
   TX_Yellow_on_Cyan,                           // cViewTitleStand
   TX_Lwhite_on_Magenta,                        // cViewTitleFocus
   TX_Cyan_on_Cyan,                             // cViewFooterStand
   TX_Yellow_on_Blue,                           // cViewFooterFocus
   TX_White_on_Cyan,                            // cSbvBorder_top
   TX_White_on_Cyan,                            // cSbvBorder_trc
   TX_White_on_Cyan,                            // cSbvBorder_rgt
   TX_White_on_Cyan,                            // cSbvBorder_brc
   TX_White_on_Cyan,                            // cSbvBorder_bot
   TX_White_on_Cyan,                            // cSbvBorder_blc
   TX_White_on_Cyan,                            // cSbvBorder_lft
   TX_White_on_Cyan,                            // cSbvBorder_tlc
   TX_Yellow_on_Cyan,                           // cSbvTitleStand
   TX_Lwhite_on_Magenta,                        // cSbvTitleFocus
   TX_White_on_Cyan,                            // cSbvFooterStand
   TX_White_on_Cyan,                            // cSbvFooterFocus
   TX_Yellow_on_Cyan,                           // cSbvStatusStand
   TX_Yellow_on_Cyan,                           // cSbvStatusFocus
   TX_Lgreen_on_Cyan,                           // cSbvProgreStand
   TX_Lgreen_on_Cyan,                           // cSbvTraceStand
   TX_White_on_Magenta,                         // cSbvMarkedArea
   TX_Lgreen_on_Cyan,                           // cDskTraceStand
   TX_White_on_Cyan,                            // cDskBorder_top
   TX_White_on_Cyan,                            // cDskBorder_trc
   TX_White_on_Cyan,                            // cDskBorder_rgt
   TX_White_on_Cyan,                            // cDskBorder_brc
   TX_White_on_Cyan,                            // cDskBorder_bot
   TX_White_on_Cyan,                            // cDskBorder_blc
   TX_White_on_Cyan,                            // cDskBorder_lft
   TX_White_on_Cyan,                            // cDskBorder_tlc
   TX_Lwhite_on_Cyan,                           // cDskTitleStand
   TX_Lwhite_on_Cyan,                           // cDskTitleFocus
   TX_White_on_Blue,                            // cDskFooterStand
   TX_White_on_Blue,                            // cDskFooterFocus
   TX_Black_on_White,                           // cLineTextStand
   TX_Black_on_White,                           // cLineTextNr0
   TX_Blue_on_White,                            // cLineTextNr1
   TX_Green_on_White,                           // cLineTextNr2
   TX_Cyan_on_White,                            // cLineTextNr3
   TX_Red_on_White,                             // cLineTextNr4
   TX_Magenta_on_White,                         // cLineTextNr5
   TX_Yellow_on_White,                          // cLineTextNr6
   TX_Grey_on_White,                            // cLineTextNr7
   TX_Black_on_White,                           // cHelpTextStand
   TX_Black_on_White,                           // cHelpTextHelp
   TX_Black_on_White,                           // cViewTextStand
   TX_White_on_Magenta,                         // cViewTextMark
   TX_Blue_on_White,                            // cViewTextNr1
   TX_Green_on_White,                           // cViewTextNr2
   TX_Cyan_on_White,                            // cViewTextNr3
   TX_Red_on_White,                             // cViewTextNr4
   TX_Magenta_on_White,                         // cViewTextNr5
   TX_Yellow_on_White,                          // cViewTextNr6
   TX_Grey_on_White,                            // cViewTextNr7
   TX_Black_on_White,                           // cListTextStand
   TX_Green_on_Black,                           // cListTextFocus
   TX_Green_on_White,                           // cListSelectStand
   TX_Lgreen_on_Black,                          // cListSelectFocus
   TX_Magenta_on_White,                         // cListMarkStand
   TX_Magenta_on_Black,                         // cListMarkFocus
   TX_Lwhite_on_White,                          // cListDisableStand
   TX_White_on_Blue,                            // cListDisableFocus
   TX_Black_on_White,                           // cListSeparatStand
   TX_Green_on_Black,                           // cListSeparatFocus
   TX_White_on_Cyan,                            // cListCountStand
   TX_Yellow_on_Cyan,                           // cListCountFocus
   TX_Black_on_White,                           // cFileTextStand
   TX_Green_on_Black,                           // cFileTextFocus
   TX_Black_on_White,                           // cFileSelectStand
   TX_Green_on_Black,                           // cFileSelectFocus
   TX_Magenta_on_White,                         // cFileMarkStand
   TX_Magenta_on_Black,                         // cFileMarkFocus
   TX_Lwhite_on_White,                          // cFileDisableStand
   TX_White_on_Blue,                            // cFileDisableFocus
   TX_Black_on_White,                           // cFileSeparatStand
   TX_Green_on_Black,                           // cFileSeparatFocus
   TX_White_on_Cyan,                            // cFileCountStand
   TX_Yellow_on_Cyan,                           // cFileCountFocus
   TX_White_on_Cyan,                            // cFileBorder_top
   TX_White_on_Cyan,                            // cFileBorder_trc
   TX_White_on_Cyan,                            // cFileBorder_rgt
   TX_White_on_Cyan,                            // cFileBorder_brc
   TX_White_on_Cyan,                            // cFileBorder_bot
   TX_White_on_Cyan,                            // cFileBorder_blc
   TX_White_on_Cyan,                            // cFileBorder_lft
   TX_White_on_Cyan,                            // cFileBorder_tlc
   TX_Yellow_on_Cyan,                           // cFileTitleStand
   TX_Lwhite_on_Magenta,                        // cFileTitleFocus
   TX_Cyan_on_Cyan,                             // cFileFooterStand
   TX_Yellow_on_Blue,                           // cFileFooterFocus
   TX_Black_on_White,                           // cApplTextStand
   TX_Green_on_Black,                           // cApplTextFocus
   TX_Green_on_White,                           // cApplSelectStand
   TX_Lgreen_on_Black,                          // cApplSelectFocus
   TX_Magenta_on_White,                         // cApplMarkStand
   TX_Magenta_on_Black,                         // cApplMarkFocus
   TX_Lwhite_on_White,                          // cApplDisableStand
   TX_White_on_Blue,                            // cApplDisableFocus
   TX_Black_on_White,                           // cApplSeparatStand
   TX_Green_on_Black,                           // cApplSeparatFocus
   TX_White_on_Grey,                            // cMenuTextStand
   TX_White_on_Blue,                            // cMenuTextFocus
   TX_Yellow_on_Grey,                           // cMenuSelectStand
   TX_Yellow_on_Blue,                           // cMenuSelectFocus
   TX_Black_on_Grey,                            // cMenuMarkStand
   TX_Cyan_on_Blue,                             // cMenuMarkFocus            (mbar)
   TX_Black_on_Grey,                            // cMenuDisableStand
   TX_Cyan_on_Blue,                             // cMenuDisableFocus         (mbar)
   TX_White_on_Grey,                            // cMenuSeparatStand
   TX_White_on_Blue,                            // cMenuSeparatFocus
   TX_White_on_Cyan,                            // cMenuBorder_top
   TX_White_on_Cyan,                            // cMenuBorder_trc
   TX_White_on_Cyan,                            // cMenuBorder_rgt
   TX_White_on_Cyan,                            // cMenuBorder_brc
   TX_White_on_Cyan,                            // cMenuBorder_bot
   TX_White_on_Cyan,                            // cMenuBorder_blc
   TX_White_on_Cyan,                            // cMenuBorder_lft
   TX_White_on_Cyan,                            // cMenuBorder_tlc
   TX_White_on_Grey,                            // cMbarBorder_top
   TX_White_on_Grey,                            // cMbarBorder_trc
   TX_White_on_Grey,                            // cMbarBorder_rgt
   TX_White_on_Grey,                            // cMbarBorder_brc
   TX_White_on_Grey,                            // cMbarBorder_bot
   TX_White_on_Grey,                            // cMbarBorder_blc
   TX_White_on_Grey,                            // cMbarBorder_lft
   TX_White_on_Grey,                            // cMbarBorder_tlc
   TX_White_on_Grey,                            // cMbarTextStand
   TX_White_on_Grey,                            // cMbarTextFocus
   TX_Yellow_on_Grey,                           // cMbarHeadStand
   TX_Lwhite_on_Magenta,                        // cMbarHeadFocus
   TX_White_on_Grey,                            // cSpinTextStand
   TX_Lwhite_on_Grey,                           // cSpinTextFocus
   TX_Yellow_on_Grey,                           // cSpinSelectStand
   TX_Yellow_on_Grey,                           // cSpinSelectFocus
   TX_Black_on_Grey,                            // cSpinMarkStand
   TX_Black_on_Grey,                            // cSpinMarkFocus
   TX_Black_on_Grey,                            // cSpinDisableStand
   TX_Black_on_Grey,                            // cSpinDisableFocus
   TX_White_on_Grey,                            // cSpinSeparatStand
   TX_White_on_Grey,                            // cSpinSeparatFocus
   TX_Grey_on_Grey,                             // cSpinIndcStand
   TX_White_on_Blue,                            // cSpinIndcFocus
   TX_Lwhite_on_Grey,                           // cPushTextStand
   TX_Yellow_on_Grey,                           // cPushTextFocus
   TX_Black_on_White,                           // cRadioValueStand
   TX_Black_on_White,                           // cRadioValueFocus
   TX_Black_on_White,                           // cRadioTextStand
   TX_Black_on_White,                           // cRadioTextFocus
   TX_Black_on_White,                           // cCheckValueStand
   TX_Black_on_White,                           // cCheckValueFocus
   TX_Black_on_White,                           // cCheckTextStand
   TX_Black_on_White,                           // cCheckTextFocus
   TX_Yellow_on_Green,                          // cDlgEfTextStand
   TX_Lwhite_on_Green,                          // cDlgEfTextFocus
   TX_Yellow_on_Green,                          // cEntryTextStand
   TX_Yellow_on_Green,                          // cEntryTextFocus
   TX_Green_on_Green,                           // cEntryHistStand
   TX_White_on_Green,                           // cEntryHistFocus
   TX_White_on_Green,                           // cEntrBorder_top
   TX_White_on_Green,                           // cEntrBorder_trc
   TX_White_on_Green,                           // cEntrBorder_rgt
   TX_White_on_Green,                           // cEntrBorder_brc
   TX_White_on_Green,                           // cEntrBorder_bot
   TX_White_on_Green,                           // cEntrBorder_blc
   TX_White_on_Green,                           // cEntrBorder_lft
   TX_White_on_Green,                           // cEntrBorder_tlc
   TX_White_on_Green,                           // cEntrTitleStand
   TX_Lwhite_on_Magenta,                        // cEntrTitleFocus
   TX_White_on_Green,                           // cEntrFooterStand
   TX_White_on_Blue,                            // cEntrFooterFocus
   TX_Black_on_White,                           // cMLEntTextStand
   TX_Black_on_White,                           // cMLEntTextFocus
   TX_Yellow_on_Magenta,                        // cHexEdCurByteChg
   TX_Lwhite_on_Magenta,                        // cHexEdCursorByte
   TX_White_on_Blue,                            // cHexEdHexByteStd
   TX_Yellow_on_Blue,                           // cHexEdHexByteChg
   TX_White_on_Magenta,                         // cHexEdHexByteMrk
   TX_Yellow_on_Magenta,                        // cHexEdHexByteMch
   TX_White_on_Blue,                            // cHexEdAscByteStd
   TX_Lcyan_on_Blue,                            // cHexEdAscBracket
   TX_Brown_on_Blue,                            // cHexEdRelPosPrev
   TX_Lgreen_on_Blue,                           // cHexEdRelPosCurr
   TX_Cyan_on_Blue,                             // cHexEdRelPosNext
   TX_Green_on_Blue,                            // cHexEdAbsBytePos
   TX_Green_on_White,                           // cHexEdAbsByteCur
   TX_Green_on_White,                           // cHexEdRelCursorP
   TX_Yellow_on_Grey,                           // cHexEdButtonText
   TX_White_on_Grey,                            // cHexEdButBracket
   TX_Grey_on_White,                            // cHexEdByteNumber
   TX_White_on_Magenta,                         // cHexEdItemSnText
   TX_Lwhite_on_Magenta,                        // cHexEdItemHlight
   TX_Red_on_White,                             // cHexEdModifyText
};

//- A mainly Black & White scheme with a 3D-look using White menus and Windows
static TXW_COLORMAP    txwcm_3w =
{
   0,                                           // cSchemeColor
   TX_Black_on_White,                           // cWinClientClr
   TX_Black_on_White,                           // cWinClientNr0
   TX_Black_on_White,                           // cWinClientNr1
   TX_Black_on_White,                           // cWinClientNr2
   TX_Black_on_White,                           // cWinClientNr3
   TX_Black_on_White,                           // cWinClientNr4
   TX_Black_on_White,                           // cWinClientNr5
   TX_Black_on_White,                           // cWinClientNr6
   TX_Black_on_White,                           // cWinClientNr7
   TX_Lwhite_on_White,                          // cWinBorder_top
   TX_Lwhite_on_White,                          // cWinBorder_trc
   TX_Black_on_White,                           // cWinBorder_rgt
   TX_Black_on_White,                           // cWinBorder_brc
   TX_Black_on_White,                           // cWinBorder_bot
   TX_Lwhite_on_White,                          // cWinBorder_blc
   TX_Lwhite_on_White,                          // cWinBorder_lft
   TX_Lwhite_on_White,                          // cWinBorder_tlc
   TX_Black_on_White,                           // cWinTitleStand
   TX_Lwhite_on_White,                          // cWinTitleFocus
   TX_Black_on_White,                           // cWinFooterStand
   TX_Lwhite_on_White,                          // cWinFooterFocus
   TX_Lwhite_on_White,                          // cDlgBorder_top
   TX_Lwhite_on_White,                          // cDlgBorder_trc
   TX_Black_on_White,                           // cDlgBorder_rgt
   TX_Black_on_White,                           // cDlgBorder_brc
   TX_Black_on_White,                           // cDlgBorder_bot
   TX_Lwhite_on_White,                          // cDlgBorder_blc
   TX_Lwhite_on_White,                          // cDlgBorder_lft
   TX_Lwhite_on_White,                          // cDlgBorder_tlc
   TX_Black_on_White,                           // cDlgTitleStand
   TX_Lwhite_on_White,                          // cDlgTitleFocus
   TX_Black_on_White,                           // cDlgFooterStand
   TX_Lwhite_on_White,                          // cDlgFooterFocus
   TX_Lwhite_on_White,                          // cPushBorder_top
   TX_Lwhite_on_White,                          // cPushBorder_trc
   TX_Black_on_White,                           // cPushBorder_rgt
   TX_Black_on_White,                           // cPushBorder_brc
   TX_Black_on_White,                           // cPushBorder_bot
   TX_Lwhite_on_White,                          // cPushBorder_blc
   TX_Lwhite_on_White,                          // cPushBorder_lft
   TX_Lwhite_on_White,                          // cPushBorder_tlc
   TX_Black_on_White,                           // cPushTitleStand
   TX_Lwhite_on_White,                          // cPushTitleFocus
   TX_Black_on_White,                           // cPushFooterStand
   TX_Lwhite_on_White,                          // cPushFooterFocus
   TX_Lwhite_on_White,                          // cViewBorder_top
   TX_Lwhite_on_White,                          // cViewBorder_trc
   TX_Black_on_White,                           // cViewBorder_rgt
   TX_Black_on_White,                           // cViewBorder_brc
   TX_Black_on_White,                           // cViewBorder_bot
   TX_Lwhite_on_White,                          // cViewBorder_blc
   TX_Lwhite_on_White,                          // cViewBorder_lft
   TX_Lwhite_on_White,                          // cViewBorder_tlc
   TX_Black_on_White,                           // cViewTitleStand
   TX_Lwhite_on_White,                          // cViewTitleFocus
   TX_Black_on_White,                           // cViewFooterStand
   TX_Lwhite_on_White,                          // cViewFooterFocus
   TX_Black_on_White,                           // cSbvBorder_top
   TX_White_on_White,                           // cSbvBorder_trc (or blank)
   TX_Lwhite_on_White,                          // cSbvBorder_rgt
   TX_Lwhite_on_White,                          // cSbvBorder_brc
   TX_Lwhite_on_White,                          // cSbvBorder_bot
   TX_White_on_White,                           // cSbvBorder_blc (or blank)
   TX_Black_on_White,                           // cSbvBorder_lft
   TX_Black_on_White,                           // cSbvBorder_tlc
   TX_Black_on_White,                           // cSbvTitleStand
   TX_Lwhite_on_White,                          // cSbvTitleFocus
   TX_Black_on_White,                           // cSbvFooterStand
   TX_Black_on_White,                           // cSbvFooterFocus
   TX_Black_on_White,                           // cSbvStatusStand
   TX_Black_on_White,                           // cSbvStatusFocus
   TX_Black_on_White,                           // cSbvProgreStand
   TX_Lwhite_on_Black,                          // cSbvTraceStand
   TX_Black_on_White,                           // cSbvMarkedArea
   TX_Lwhite_on_Black,                          // cDskTraceStand
   TX_Lwhite_on_White,                          // cDskBorder_top
   TX_White_on_White,                           // cDskBorder_trc (or blank)
   TX_Black_on_White,                           // cDskBorder_rgt
   TX_Black_on_White,                           // cDskBorder_brc
   TX_Black_on_White,                           // cDskBorder_bot
   TX_White_on_White,                           // cDskBorder_blc (or blank)
   TX_Lwhite_on_White,                          // cDskBorder_lft
   TX_Lwhite_on_White,                          // cDskBorder_tlc
   TX_Black_on_White,                           // cDskTitleStand
   TX_Lwhite_on_White,                          // cDskTitleFocus
   TX_Black_on_White,                           // cDskFooterStand
   TX_Black_on_White,                           // cDskFooterFocus
   TX_Black_on_White,                           // cLineTextStand
   TX_Black_on_White,                           // cLineTextNr0
   TX_Black_on_White,                           // cLineTextNr1
   TX_Black_on_White,                           // cLineTextNr2
   TX_Black_on_White,                           // cLineTextNr3
   TX_Black_on_White,                           // cLineTextNr4
   TX_Black_on_White,                           // cLineTextNr5
   TX_Black_on_White,                           // cLineTextNr6
   TX_Black_on_White,                           // cLineTextNr7
   TX_Black_on_White,                           // cHelpTextStand
   TX_Black_on_White,                           // cHelpTextHelp
   TX_Black_on_White,                           // cViewTextStand
   TX_Lwhite_on_Black,                          // cViewTextMark
   TX_Black_on_White,                           // cViewTextNr1
   TX_Black_on_White,                           // cViewTextNr2
   TX_Black_on_White,                           // cViewTextNr3
   TX_Black_on_White,                           // cViewTextNr4
   TX_Black_on_White,                           // cViewTextNr5
   TX_Black_on_White,                           // cViewTextNr6
   TX_Black_on_White,                           // cViewTextNr7
   TX_Black_on_White,                           // cListTextStand
   TX_White_on_Black,                           // cListTextFocus
   TX_Lwhite_on_Black,                          // cListSelectStand
   TX_Lwhite_on_Black,                          // cListSelectFocus
   TX_Black_on_White,                           // cListMarkStand
   TX_White_on_Black,                           // cListMarkFocus
   TX_Black_on_White,                           // cListDisableStand
   TX_White_on_Black,                           // cListDisableFocus
   TX_Lwhite_on_White,                          // cListSeparatStand
   TX_Lwhite_on_White,                          // cListSeparatFocus
   TX_Lwhite_on_Black,                          // cListCountStand
   TX_Lwhite_on_Black,                          // cListCountFocus
   TX_Black_on_White,                           // cFileTextStand
   TX_White_on_Black,                           // cFileTextFocus
   TX_Black_on_White,                           // cFileSelectStand
   TX_White_on_Black,                           // cFileSelectFocus
   TX_Black_on_White,                           // cFileMarkStand
   TX_White_on_Black,                           // cFileMarkFocus
   TX_Black_on_White,                           // cFileDisableStand
   TX_White_on_Black,                           // cFileDisableFocus
   TX_Lwhite_on_White,                          // cFileSeparatStand
   TX_Lwhite_on_White,                          // cFileSeparatFocus
   TX_Lwhite_on_Black,                          // cFileCountStand
   TX_Lwhite_on_Black,                          // cFileCountFocus
   TX_Black_on_White,                           // cFileBorder_top
   TX_White_on_White,                           // cFileBorder_trc
   TX_Lwhite_on_White,                          // cFileBorder_rgt
   TX_Lwhite_on_White,                          // cFileBorder_brc
   TX_Lwhite_on_White,                          // cFileBorder_bot
   TX_White_on_White,                           // cFileBorder_blc
   TX_Black_on_White,                           // cFileBorder_lft
   TX_Black_on_White,                           // cFileBorder_tlc
   TX_White_on_Black,                           // cFileTitleStand
   TX_Lwhite_on_Black,                          // cFileTitleFocus
   TX_White_on_Black,                           // cFileFooterStand
   TX_Lwhite_on_Black,                          // cFileFooterFocus
   TX_Black_on_White,                           // cApplTextStand
   TX_White_on_Black,                           // cApplTextFocus
   TX_Lwhite_on_Black,                          // cApplSelectStand
   TX_Lwhite_on_Black,                          // cApplSelectFocus
   TX_Black_on_White,                           // cApplMarkStand
   TX_White_on_Black,                           // cApplMarkFocus
   TX_Black_on_White,                           // cApplDisableStand
   TX_White_on_Black,                           // cApplDisableFocus
   TX_Lwhite_on_White,                          // cApplSeparatStand
   TX_Lwhite_on_White,                          // cApplSeparatFocus
   TX_Black_on_White,                           // cMenuTextStand
   TX_White_on_Black,                           // cMenuTextFocus
   TX_Lwhite_on_White,                          // cMenuSelectStand
   TX_Lwhite_on_Black,                          // cMenuSelectFocus
   TX_Black_on_White,                           // cMenuMarkStand
   TX_White_on_Black,                           // cMenuMarkFocus
   TX_Black_on_White,                           // cMenuDisableStand
   TX_White_on_Black,                           // cMenuDisableFocus
   TX_Lwhite_on_White,                          // cMenuSeparatStand
   TX_Lwhite_on_White,                          // cMenuSeparatFocus
   TX_Lwhite_on_White,                          // cMenuBorder_top
   TX_Lwhite_on_White,                          // cMenuBorder_trc
   TX_Black_on_White,                           // cMenuBorder_rgt
   TX_Black_on_White,                           // cMenuBorder_brc
   TX_Black_on_White,                           // cMenuBorder_bot
   TX_Lwhite_on_White,                          // cMenuBorder_blc
   TX_Lwhite_on_White,                          // cMenuBorder_lft
   TX_Lwhite_on_White,                          // cMenuBorder_tlc
   TX_Black_on_White,                           // cMbarBorder_top
   TX_Black_on_White,                           // cMbarBorder_trc
   TX_Black_on_White,                           // cMbarBorder_rgt
   TX_Black_on_White,                           // cMbarBorder_brc
   TX_Black_on_White,                           // cMbarBorder_bot
   TX_Black_on_White,                           // cMbarBorder_blc
   TX_Black_on_White,                           // cMbarBorder_lft
   TX_Black_on_White,                           // cMbarBorder_tlc
   TX_Black_on_White,                           // cMbarTextStand
   TX_Black_on_White,                           // cMbarTextFocus
   TX_Black_on_White,                           // cMbarHeadStand
   TX_Lwhite_on_Black,                          // cMbarHeadFocus
   TX_Black_on_White,                           // cSpinTextStand
   TX_White_on_Black,                           // cSpinTextFocus
   TX_Lwhite_on_Black,                          // cSpinSelectStand
   TX_Lwhite_on_Black,                          // cSpinSelectFocus
   TX_Black_on_White,                           // cSpinMarkStand
   TX_White_on_Black,                           // cSpinMarkFocus
   TX_Black_on_White,                           // cSpinDisableStand
   TX_White_on_Black,                           // cSpinDisableFocus
   TX_Lwhite_on_White,                          // cSpinSeparatStand
   TX_Lwhite_on_White,                          // cSpinSeparatFocus
   TX_White_on_Black,                           // cSpinIndcStand
   TX_Black_on_White,                           // cSpinIndcFocus
   TX_Black_on_White,                           // cPushTextStand
   TX_White_on_Black,                           // cPushTextFocus
   TX_Black_on_White,                           // cRadioValueStand
   TX_Black_on_White,                           // cRadioValueFocus
   TX_Black_on_White,                           // cRadioTextStand
   TX_Black_on_White,                           // cRadioTextFocus
   TX_Black_on_White,                           // cCheckValueStand
   TX_Black_on_White,                           // cCheckValueFocus
   TX_Black_on_White,                           // cCheckTextStand
   TX_Black_on_White,                           // cCheckTextFocus
   TX_White_on_Black,                           // cDlgEfTextStand
   TX_Lwhite_on_Black,                          // cDlgEfTextFocus
   TX_White_on_Black,                           // cEntryTextStand
   TX_Lwhite_on_Black,                          // cEntryTextFocus
   TX_Black_on_Black,                           // cEntryHistStand
   TX_White_on_Black,                           // cEntryHistFocus
   TX_Lwhite_on_White,                          // cEntrBorder_top
   TX_Lwhite_on_White,                          // cEntrBorder_trc
   TX_Black_on_White,                           // cEntrBorder_rgt
   TX_Black_on_White,                           // cEntrBorder_brc
   TX_Black_on_White,                           // cEntrBorder_bot
   TX_Lwhite_on_White,                          // cEntrBorder_blc
   TX_Lwhite_on_White,                          // cEntrBorder_lft
   TX_Lwhite_on_White,                          // cEntrBorder_tlc
   TX_White_on_Black,                           // cEntrTitleStand
   TX_Lwhite_on_Black,                          // cEntrTitleFocus
   TX_White_on_Black,                           // cEntrFooterStand
   TX_Lwhite_on_Black,                          // cEntrFooterFocus
   TX_Black_on_White,                           // cMLEntTextStand
   TX_Black_on_White,                           // cMLEntTextFocus
   TX_Lwhite_on_Black,                          // cHexEdCurByteChg
   TX_White_on_Black,                           // cHexEdCursorByte
   TX_Black_on_White,                           // cHexEdHexByteStd
   TX_Lwhite_on_White,                          // cHexEdHexByteChg
   TX_Lwhite_on_Black,                          // cHexEdHexByteMrk
   TX_Lwhite_on_White,                          // cHexEdHexByteMch
   TX_Black_on_White,                           // cHexEdAscByteStd
   TX_Lwhite_on_White,                          // cHexEdAscBracket
   TX_White_on_Black,                           // cHexEdRelPosPrev
   TX_Lwhite_on_Black,                          // cHexEdRelPosCurr
   TX_White_on_Black,                           // cHexEdRelPosNext
   TX_White_on_Black,                           // cHexEdAbsBytePos
   TX_White_on_Black,                           // cHexEdAbsByteCur
   TX_White_on_Black,                           // cHexEdRelCursorP
   TX_Lwhite_on_Black,                          // cHexEdButtonText
   TX_White_on_Black,                           // cHexEdButBracket
   TX_White_on_Black,                           // cHexEdByteNumber
   TX_White_on_Black,                           // cHexEdItemSnText
   TX_Lwhite_on_Black,                          // cHexEdItemHlight
   TX_Lwhite_on_Black,                          // cHexEdModifyText
};

//- Black & White scheme with Black menus and White Windows, for MONOCHROME use
static TXW_COLORMAP    txwcm_bw =
{
   0,                                           // cSchemeColor
   TX_Black_on_White,                           // cWinClientClr
   TX_Black_on_White,                           // cWinClientNr0
   TX_Black_on_White,                           // cWinClientNr1
   TX_Black_on_White,                           // cWinClientNr2
   TX_Black_on_White,                           // cWinClientNr3
   TX_Black_on_White,                           // cWinClientNr4
   TX_Black_on_White,                           // cWinClientNr5
   TX_Black_on_White,                           // cWinClientNr6
   TX_Black_on_White,                           // cWinClientNr7
   TX_Black_on_White,                           // cWinBorder_top
   TX_Black_on_White,                           // cWinBorder_trc
   TX_Black_on_White,                           // cWinBorder_rgt
   TX_Black_on_White,                           // cWinBorder_brc
   TX_Black_on_White,                           // cWinBorder_bot
   TX_Black_on_White,                           // cWinBorder_blc
   TX_Black_on_White,                           // cWinBorder_lft
   TX_Black_on_White,                           // cWinBorder_tlc
   TX_Black_on_White,                           // cWinTitleStand
   TX_Black_on_White,                           // cWinTitleFocus
   TX_Black_on_White,                           // cWinFooterStand
   TX_Black_on_White,                           // cWinFooterFocus
   TX_Black_on_White,                           // cDlgBorder_top
   TX_Black_on_White,                           // cDlgBorder_trc
   TX_Black_on_White,                           // cDlgBorder_rgt
   TX_Black_on_White,                           // cDlgBorder_brc
   TX_Black_on_White,                           // cDlgBorder_bot
   TX_Black_on_White,                           // cDlgBorder_blc
   TX_Black_on_White,                           // cDlgBorder_lft
   TX_Black_on_White,                           // cDlgBorder_tlc
   TX_Black_on_White,                           // cDlgTitleStand
   TX_Black_on_White,                           // cDlgTitleFocus
   TX_Black_on_White,                           // cDlgFooterStand
   TX_Black_on_White,                           // cDlgFooterFocus
   TX_Black_on_White,                           // cPushBorder_top
   TX_Black_on_White,                           // cPushBorder_trc
   TX_Black_on_White,                           // cPushBorder_rgt
   TX_Black_on_White,                           // cPushBorder_brc
   TX_Black_on_White,                           // cPushBorder_bot
   TX_Black_on_White,                           // cPushBorder_blc
   TX_Black_on_White,                           // cPushBorder_lft
   TX_Black_on_White,                           // cPushBorder_tlc
   TX_Black_on_White,                           // cPushTitleStand
   TX_Black_on_White,                           // cPushTitleFocus
   TX_Black_on_White,                           // cPushFooterStand
   TX_Black_on_White,                           // cPushFooterFocus
   TX_Black_on_White,                           // cViewBorder_top
   TX_Black_on_White,                           // cViewBorder_trc
   TX_Black_on_White,                           // cViewBorder_rgt
   TX_Black_on_White,                           // cViewBorder_brc
   TX_Black_on_White,                           // cViewBorder_bot
   TX_Black_on_White,                           // cViewBorder_blc
   TX_Black_on_White,                           // cViewBorder_lft
   TX_Black_on_White,                           // cViewBorder_tlc
   TX_Black_on_White,                           // cViewTitleStand
   TX_Black_on_White,                           // cViewTitleFocus
   TX_Black_on_White,                           // cViewFooterStand
   TX_Black_on_White,                           // cViewFooterFocus
   TX_Black_on_White,                           // cSbvBorder_top
   TX_Black_on_White,                           // cSbvBorder_trc
   TX_Black_on_White,                           // cSbvBorder_rgt
   TX_Black_on_White,                           // cSbvBorder_brc
   TX_Black_on_White,                           // cSbvBorder_bot
   TX_Black_on_White,                           // cSbvBorder_blc
   TX_Black_on_White,                           // cSbvBorder_lft
   TX_Black_on_White,                           // cSbvBorder_tlc
   TX_Black_on_White,                           // cSbvTitleStand
   TX_Black_on_White,                           // cSbvTitleFocus
   TX_Black_on_White,                           // cSbvFooterStand
   TX_Black_on_White,                           // cSbvFooterFocus
   TX_Black_on_White,                           // cSbvStatusStand
   TX_Black_on_White,                           // cSbvStatusFocus
   TX_Lwhite_on_White,                          // cSbvProgreStand
   TX_Lwhite_on_White,                          // cSbvTraceStand
   TX_Black_on_White,                           // cSbvMarkedArea
   TX_Black_on_White,                           // cDskTraceStand
   TX_Black_on_White,                           // cDskBorder_top
   TX_Black_on_White,                           // cDskBorder_trc
   TX_Black_on_White,                           // cDskBorder_rgt
   TX_Black_on_White,                           // cDskBorder_brc
   TX_Black_on_White,                           // cDskBorder_bot
   TX_Black_on_White,                           // cDskBorder_blc
   TX_Black_on_White,                           // cDskBorder_lft
   TX_Black_on_White,                           // cDskBorder_tlc
   TX_Black_on_White,                           // cDskTitleStand
   TX_Black_on_White,                           // cDskTitleFocus
   TX_Black_on_White,                           // cDskFooterStand
   TX_Black_on_White,                           // cDskFooterFocus
   TX_Black_on_White,                           // cLineTextStand
   TX_Black_on_White,                           // cLineTextNr0
   TX_Black_on_White,                           // cLineTextNr1
   TX_Black_on_White,                           // cLineTextNr2
   TX_Black_on_White,                           // cLineTextNr3
   TX_Black_on_White,                           // cLineTextNr4
   TX_Black_on_White,                           // cLineTextNr5
   TX_Black_on_White,                           // cLineTextNr6
   TX_Black_on_White,                           // cLineTextNr7
   TX_Black_on_White,                           // cHelpTextStand
   TX_Black_on_White,                           // cHelpTextHelp
   TX_Black_on_White,                           // cViewTextStand
   TX_Lwhite_on_Black,                          // cViewTextMark
   TX_Black_on_White,                           // cViewTextNr1
   TX_Black_on_White,                           // cViewTextNr2
   TX_Black_on_White,                           // cViewTextNr3
   TX_Black_on_White,                           // cViewTextNr4
   TX_Black_on_White,                           // cViewTextNr5
   TX_Black_on_White,                           // cViewTextNr6
   TX_Black_on_White,                           // cViewTextNr7
   TX_Black_on_White,                           // cListTextStand
   TX_White_on_Black,                           // cListTextFocus
   TX_Lwhite_on_Black,                          // cListSelectStand
   TX_Lwhite_on_Black,                          // cListSelectFocus
   TX_Black_on_White,                           // cListMarkStand
   TX_White_on_Black,                           // cListMarkFocus
   TX_Black_on_White,                           // cListDisableStand
   TX_White_on_Black,                           // cListDisableFocus
   TX_Lwhite_on_White,                          // cListSeparatStand
   TX_Lwhite_on_White,                          // cListSeparatFocus
   TX_Lwhite_on_Black,                          // cListCountStand
   TX_Lwhite_on_Black,                          // cListCountFocus
   TX_Black_on_White,                           // cFileTextStand
   TX_White_on_Black,                           // cFileTextFocus
   TX_Black_on_White,                           // cFileSelectStand
   TX_White_on_Black,                           // cFileSelectFocus
   TX_Black_on_White,                           // cFileMarkStand
   TX_White_on_Black,                           // cFileMarkFocus
   TX_Black_on_White,                           // cFileDisableStand
   TX_White_on_Black,                           // cFileDisableFocus
   TX_Lwhite_on_White,                          // cFileSeparatStand
   TX_Lwhite_on_White,                          // cFileSeparatFocus
   TX_Lwhite_on_Black,                          // cFileCountStand
   TX_Lwhite_on_Black,                          // cFileCountFocus
   TX_Black_on_White,                           // cFileBorder_top
   TX_Black_on_White,                           // cFileBorder_trc
   TX_Black_on_White,                           // cFileBorder_rgt
   TX_Black_on_White,                           // cFileBorder_brc
   TX_Black_on_White,                           // cFileBorder_bot
   TX_Black_on_White,                           // cFileBorder_blc
   TX_Black_on_White,                           // cFileBorder_lft
   TX_Black_on_White,                           // cFileBorder_tlc
   TX_Black_on_White,                           // cFileTitleStand
   TX_Lwhite_on_Black,                          // cFileTitleFocus
   TX_White_on_Black,                           // cFileFooterStand
   TX_Lwhite_on_Black,                          // cFileFooterFocus
   TX_Black_on_White,                           // cApplTextStand
   TX_White_on_Black,                           // cApplTextFocus
   TX_Lwhite_on_Black,                          // cApplSelectStand
   TX_Lwhite_on_Black,                          // cApplSelectFocus
   TX_Black_on_White,                           // cApplMarkStand
   TX_White_on_Black,                           // cApplMarkFocus
   TX_Black_on_White,                           // cApplDisableStand
   TX_White_on_Black,                           // cApplDisableFocus
   TX_Lwhite_on_White,                          // cApplSeparatStand
   TX_Lwhite_on_White,                          // cApplSeparatFocus
   TX_White_on_Black,                           // cMenuTextStand
   TX_Black_on_White,                           // cMenuTextFocus
   TX_Lwhite_on_Black,                          // cMenuSelectStand
   TX_Lwhite_on_White,                          // cMenuSelectFocus
   TX_White_on_Black,                           // cMenuMarkStand
   TX_Black_on_White,                           // cMenuMarkFocus
   TX_White_on_Black,                           // cMenuDisableStand
   TX_Black_on_White,                           // cMenuDisableFocus
   TX_White_on_Black,                           // cMenuSeparatStand
   TX_Black_on_White,                           // cMenuSeparatFocus
   TX_White_on_Black,                           // cMenuBorder_top
   TX_White_on_Black,                           // cMenuBorder_trc
   TX_White_on_Black,                           // cMenuBorder_rgt
   TX_White_on_Black,                           // cMenuBorder_brc
   TX_White_on_Black,                           // cMenuBorder_bot
   TX_White_on_Black,                           // cMenuBorder_blc
   TX_White_on_Black,                           // cMenuBorder_lft
   TX_White_on_Black,                           // cMenuBorder_tlc
   TX_Black_on_White,                           // cMbarBorder_top
   TX_Black_on_White,                           // cMbarBorder_trc
   TX_Black_on_White,                           // cMbarBorder_rgt
   TX_Black_on_White,                           // cMbarBorder_brc
   TX_Black_on_White,                           // cMbarBorder_bot
   TX_Black_on_White,                           // cMbarBorder_blc
   TX_Black_on_White,                           // cMbarBorder_lft
   TX_Black_on_White,                           // cMbarBorder_tlc
   TX_Black_on_White,                           // cMbarTextStand
   TX_Black_on_White,                           // cMbarTextFocus
   TX_Black_on_White,                           // cMbarHeadStand
   TX_Lwhite_on_Black,                          // cMbarHeadFocus
   TX_Black_on_White,                           // cSpinTextStand
   TX_White_on_Black,                           // cSpinTextFocus
   TX_Lwhite_on_Black,                          // cSpinSelectStand
   TX_Lwhite_on_Black,                          // cSpinSelectFocus
   TX_Black_on_White,                           // cSpinMarkStand
   TX_White_on_Black,                           // cSpinMarkFocus
   TX_Black_on_White,                           // cSpinDisableStand
   TX_White_on_Black,                           // cSpinDisableFocus
   TX_Lwhite_on_White,                          // cSpinSeparatStand
   TX_Lwhite_on_White,                          // cSpinSeparatFocus
   TX_White_on_Black,                           // cSpinIndcStand
   TX_Black_on_White,                           // cSpinIndcFocus
   TX_Black_on_White,                           // cPushTextStand
   TX_White_on_Black,                           // cPushTextFocus
   TX_Black_on_White,                           // cRadioValueStand
   TX_Black_on_White,                           // cRadioValueFocus
   TX_Black_on_White,                           // cRadioTextStand
   TX_Black_on_White,                           // cRadioTextFocus
   TX_Black_on_White,                           // cCheckValueStand
   TX_Black_on_White,                           // cCheckValueFocus
   TX_Black_on_White,                           // cCheckTextStand
   TX_Black_on_White,                           // cCheckTextFocus
   TX_White_on_Black,                           // cDlgEfTextStand
   TX_Lwhite_on_Black,                          // cDlgEfTextFocus
   TX_White_on_Black,                           // cEntryTextStand
   TX_Lwhite_on_Black,                          // cEntryTextFocus
   TX_Black_on_Black,                           // cEntryHistStand
   TX_White_on_Black,                           // cEntryHistFocus
   TX_White_on_Black,                           // cEntrBorder_top
   TX_White_on_Black,                           // cEntrBorder_trc
   TX_White_on_Black,                           // cEntrBorder_rgt
   TX_White_on_Black,                           // cEntrBorder_brc
   TX_White_on_Black,                           // cEntrBorder_bot
   TX_White_on_Black,                           // cEntrBorder_blc
   TX_White_on_Black,                           // cEntrBorder_lft
   TX_White_on_Black,                           // cEntrBorder_tlc
   TX_White_on_Black,                           // cEntrTitleStand
   TX_Lwhite_on_Black,                          // cEntrTitleFocus
   TX_White_on_Black,                           // cEntrFooterStand
   TX_Lwhite_on_Black,                          // cEntrFooterFocus
   TX_Black_on_White,                           // cMLEntTextStand
   TX_Black_on_White,                           // cMLEntTextFocus
   TX_Lwhite_on_Black,                          // cHexEdCurByteChg
   TX_White_on_Black,                           // cHexEdCursorByte
   TX_Black_on_White,                           // cHexEdHexByteStd
   TX_Lwhite_on_White,                          // cHexEdHexByteChg
   TX_Lwhite_on_Black,                          // cHexEdHexByteMrk
   TX_Lwhite_on_White,                          // cHexEdHexByteMch
   TX_Black_on_White,                           // cHexEdAscByteStd
   TX_Lwhite_on_White,                          // cHexEdAscBracket
   TX_White_on_Black,                           // cHexEdRelPosPrev
   TX_Lwhite_on_Black,                          // cHexEdRelPosCurr
   TX_White_on_Black,                           // cHexEdRelPosNext
   TX_White_on_Black,                           // cHexEdAbsBytePos
   TX_White_on_Black,                           // cHexEdAbsByteCur
   TX_White_on_Black,                           // cHexEdRelCursorP
   TX_Lwhite_on_Black,                          // cHexEdButtonText
   TX_White_on_Black,                           // cHexEdButBracket
   TX_White_on_Black,                           // cHexEdByteNumber
   TX_White_on_Black,                           // cHexEdItemSnText
   TX_Lwhite_on_Black,                          // cHexEdItemHlight
   TX_Lwhite_on_Black,                          // cHexEdModifyText
};

//- define colorschemes using the color-maps defined above ...

static TXW_COLORSCHEME txwcs_st =
{
   TXW_CS_SIGNATURE,
   TXW_CS_STDLINES,
   TXSB_COLOR_B2BLUE | TXSB_COLOR_BRIGHT,
   TXW_CSF_NORMAL,
   "DFSee classic Cyan",
   "Classic color scheme, very close to the original fixed scheme used by DFSee",
   txwcm_st
};

static TXW_COLORSCHEME txwcs_sh =
{
   TXW_CS_SIGNATURE,
   TXW_CS_HALF,
   TXSB_COLOR_B2BLUE | TXSB_COLOR_BRIGHT,
   TXW_CSF_NORMAL,
   "DFSee classic HALF",
   "Classic color scheme, using lines of HALF size solid BLOCK characters",
   txwcm_st
};

static TXW_COLORSCHEME txwcs_sf =
{
   TXW_CS_SIGNATURE,
   TXW_CS_FULL,
   TXSB_COLOR_B2BLUE | TXSB_COLOR_BRIGHT,
   TXW_CSF_NORMAL,
   "DFSee classic FULL",
   "Classic color scheme, using FULL size solid and patterned BLOCK characters",
   txwcm_st
};

static  TXW_COLORSCHEME txwcs_3g =
{
   TXW_CS_SIGNATURE,
   TXW_CS_3D_LINES,
   TXSB_COLOR_B2BLUE | TXSB_COLOR_BRIGHT,
   TXW_CSF_NORMAL,
   "Grey 3-dimensional",
   "New default scheme, a 3D-look with Grey menus, Cyan borders and White Windows",
   txwcm_3g
};

static  TXW_COLORSCHEME txwcs_3h =
{
   TXW_CS_SIGNATURE,
   TXW_CS_HALF,
   TXSB_COLOR_B2BLUE | TXSB_COLOR_BRIGHT,
   TXW_CSF_NORMAL,
   "Grey 3D-look  HALF",
   "Grey 3D-look scheme, using lines of HALF size solid BLOCK characters",
   txwcm_3g
};

static  TXW_COLORSCHEME txwcs_3f =
{
   TXW_CS_SIGNATURE,
   TXW_CS_FULL,
   TXSB_COLOR_B2BLUE | TXSB_COLOR_BRIGHT,
   TXW_CSF_NORMAL,
   "Grey 3D-look  FULL",
   "Grey 3D-look scheme, using FULL size solid and patterned BLOCK characters",
   txwcm_3g
};

static  TXW_COLORSCHEME txwcs_nb =
{
   TXW_CS_SIGNATURE,
   TXW_CS_STDLINES,
   TXSB_COLOR_B2BLUE | TXSB_COLOR_BRIGHT,
   TXW_CSF_NORMAL,
   "Non-blinking Brown",
   "Color scheme that avoids the 'bright backgrounds' that may cause blinking",
   txwcm_nb
};

static  TXW_COLORSCHEME txwcs_cm =
{
   TXW_CS_SIGNATURE,
   TXW_CS_STDLINES,
   TXSB_COLOR_B2BLUE | TXSB_COLOR_BRIGHT,
   TXW_CSF_NORMAL,
   "Commander NC/FC/MC",
   "Color scheme inspired by the Norton Commander and clones, very Cyan/Blue",
   txwcm_cm
};

static  TXW_COLORSCHEME txwcs_ch =
{
   TXW_CS_SIGNATURE,
   TXW_CS_HALF,
   TXSB_COLOR_B2BLUE | TXSB_COLOR_BRIGHT,
   TXW_CSF_NORMAL,
   "Commander lineHALF",
   "Norton Commander scheme using lines of HALF size solid BLOCK characters",
   txwcm_cm
};

static  TXW_COLORSCHEME txwcs_cf =
{
   TXW_CS_SIGNATURE,
   TXW_CS_FULL,
   TXSB_COLOR_B2BLUE | TXSB_COLOR_BRIGHT,
   TXW_CSF_NORMAL,
   "Commander lineFULL",
   "Norton Commander scheme using FULL size solid and patterned BLOCK characters",
   txwcm_cm
};

static  TXW_COLORSCHEME txwcs_3c =
{
   TXW_CS_SIGNATURE,
   TXW_CS_3D_LINES,
   TXSB_COLOR_B2BLUE | TXSB_COLOR_BRIGHT,
   TXW_CSF_NORMAL,
   "3D-style Commander",
   "Commander scheme with a 3D-look in Cyan/Black menus and Blue/White Windows",
   txwcm_3c
};

static  TXW_COLORSCHEME txwcs_3w =
{
   TXW_CS_SIGNATURE,
   TXW_CS_3D_LINES,
   TXSB_COLOR_INVERT | TXSB_COLOR_BRIGHT,
   TXW_CSF_LIST_UNDERLINE,
   "Black and White 3D",
   "A mainly Black & White scheme with a 3D-look using White menus and Windows",
   txwcm_3w
};

static  TXW_COLORSCHEME txwcs_bw =
{
   TXW_CS_SIGNATURE,
   TXW_CS_STDLINES,
   TXSB_COLOR_INVERT | TXSB_COLOR_BRIGHT,
   TXW_CSF_LIST_UNDERLINE,
   "Black & White only",
   "Black & White scheme with Black menus and White Windows, for MONOCHROME use",
   txwcm_bw
};

static  TXW_COLORSCHEME txwcs_bh =
{
   TXW_CS_SIGNATURE,
   TXW_CS_HALF,
   TXSB_COLOR_INVERT | TXSB_COLOR_BRIGHT,
   TXW_CSF_LIST_UNDERLINE,
   "Black & White HALF",
   "Black & White scheme using HALF size solid BLOCK characters",
   txwcm_bw
};

static  TXW_COLORSCHEME txwcs_bf =
{
   TXW_CS_SIGNATURE,
   TXW_CS_FULL,
   TXSB_COLOR_INVERT | TXSB_COLOR_BRIGHT,
   TXW_CSF_LIST_UNDERLINE,
   "Black & White FULL",
   "Black & White scheme using FULL size solid and patterned BLOCK characters",
   txwcm_bw
};

//- custom colorscheme to be refined (set by APP or through INI file ?)

#endif
