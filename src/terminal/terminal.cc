/*
    Mosh: the mobile shell
    Copyright 2012 Keith Winstein

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    In addition, as a special exception, the copyright holders give
    permission to link the code of portions of this program with the
    OpenSSL library under certain conditions as described in each
    individual source file, and distribute linked combinations including
    the two.

    You must obey the GNU General Public License in all respects for all
    of the code used other than OpenSSL. If you modify file(s) with this
    exception, you may extend this exception to your version of the
    file(s), but you are not obligated to do so. If you do not wish to do
    so, delete this exception statement from your version. If you delete
    this exception statement from all source files in the program, then
    also delete it here.
*/

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <typeinfo>

#include <unistd.h>

#include "src/terminal/terminal.h"

using namespace Terminal;

#if WCHAR_MAX <= 0xFFFF
/* On systems with 16-bit wchar_t (e.g., MSYS2/Cygwin), characters above U+FFFF
   (like emoji) are represented as UTF-16 surrogate pairs. We need to track
   high surrogates to combine them with low surrogates. */
static wchar_t pending_high_surrogate_terminal = 0;

/* Calculate character width for a codepoint that may be above U+FFFF.
   On 16-bit wchar_t systems, wcwidth() can't handle values > 0xFFFF directly.
   Wide character table from wcwidth project (Unicode 17.0). */
static int wcwidth_codepoint( uint32_t codepoint )
{
  if ( codepoint <= 0xFFFF ) {
    return wcwidth( static_cast<wchar_t>( codepoint ) );
  }

  /* Wide characters in U+1F000-1FFFF (from wcwidth/table_wide.py Unicode 17.0) */
  if ( codepoint == 0x1F004 ) return 2; /* Mahjong Tile Red Dragon */
  if ( codepoint == 0x1F0CF ) return 2; /* Playing Card Black Joker */
  if ( codepoint == 0x1F18E ) return 2; /* Negative Squared Ab */
  if ( codepoint >= 0x1F191 && codepoint <= 0x1F19A ) return 2;
  if ( codepoint >= 0x1F1E6 && codepoint <= 0x1F202 ) return 2; /* Regional Indicators */
  if ( codepoint >= 0x1F210 && codepoint <= 0x1F23B ) return 2;
  if ( codepoint >= 0x1F240 && codepoint <= 0x1F248 ) return 2;
  if ( codepoint >= 0x1F250 && codepoint <= 0x1F251 ) return 2;
  if ( codepoint >= 0x1F260 && codepoint <= 0x1F265 ) return 2;
  if ( codepoint >= 0x1F300 && codepoint <= 0x1F320 ) return 2;
  if ( codepoint >= 0x1F32D && codepoint <= 0x1F335 ) return 2;
  if ( codepoint >= 0x1F337 && codepoint <= 0x1F37C ) return 2;
  if ( codepoint >= 0x1F37E && codepoint <= 0x1F393 ) return 2;
  if ( codepoint >= 0x1F3A0 && codepoint <= 0x1F3CA ) return 2;
  if ( codepoint >= 0x1F3CF && codepoint <= 0x1F3D3 ) return 2;
  if ( codepoint >= 0x1F3E0 && codepoint <= 0x1F3F0 ) return 2;
  if ( codepoint == 0x1F3F4 ) return 2; /* Waving Black Flag */
  if ( codepoint >= 0x1F3F8 && codepoint <= 0x1F43E ) return 2;
  if ( codepoint == 0x1F440 ) return 2; /* Eyes */
  if ( codepoint >= 0x1F442 && codepoint <= 0x1F4FC ) return 2;
  if ( codepoint >= 0x1F4FF && codepoint <= 0x1F53D ) return 2;
  if ( codepoint >= 0x1F54B && codepoint <= 0x1F54E ) return 2;
  if ( codepoint >= 0x1F550 && codepoint <= 0x1F567 ) return 2;
  if ( codepoint == 0x1F57A ) return 2; /* Man Dancing */
  if ( codepoint >= 0x1F595 && codepoint <= 0x1F596 ) return 2;
  if ( codepoint == 0x1F5A4 ) return 2; /* Black Heart */
  if ( codepoint >= 0x1F5FB && codepoint <= 0x1F64F ) return 2;
  if ( codepoint >= 0x1F680 && codepoint <= 0x1F6C5 ) return 2;
  if ( codepoint == 0x1F6CC ) return 2; /* Sleeping Accommodation */
  if ( codepoint >= 0x1F6D0 && codepoint <= 0x1F6D2 ) return 2;
  if ( codepoint >= 0x1F6D5 && codepoint <= 0x1F6D8 ) return 2;
  if ( codepoint >= 0x1F6DC && codepoint <= 0x1F6DF ) return 2;
  if ( codepoint >= 0x1F6EB && codepoint <= 0x1F6EC ) return 2;
  if ( codepoint >= 0x1F6F4 && codepoint <= 0x1F6FC ) return 2;
  if ( codepoint >= 0x1F7E0 && codepoint <= 0x1F7EB ) return 2; /* Colored circles/squares */
  if ( codepoint == 0x1F7F0 ) return 2; /* Heavy Equals Sign */
  if ( codepoint >= 0x1F90C && codepoint <= 0x1F93A ) return 2;
  if ( codepoint >= 0x1F93C && codepoint <= 0x1F945 ) return 2;
  if ( codepoint >= 0x1F947 && codepoint <= 0x1F9FF ) return 2;
  if ( codepoint >= 0x1FA70 && codepoint <= 0x1FA7C ) return 2;
  if ( codepoint >= 0x1FA80 && codepoint <= 0x1FA8A ) return 2;
  if ( codepoint >= 0x1FA8E && codepoint <= 0x1FAC6 ) return 2;
  if ( codepoint == 0x1FAC8 ) return 2;
  if ( codepoint >= 0x1FACD && codepoint <= 0x1FADC ) return 2;
  if ( codepoint >= 0x1FADF && codepoint <= 0x1FAEA ) return 2;
  if ( codepoint >= 0x1FAEF && codepoint <= 0x1FAF8 ) return 2;

  /* CJK Extension B and beyond */
  if ( codepoint >= 0x20000 && codepoint <= 0x2FFFF ) return 2;

  /* Everything else in supplementary planes is width 1 */
  return 1;
}
#endif

Emulator::Emulator( size_t s_width, size_t s_height ) : fb( s_width, s_height ), dispatch(), user() {}

std::string Emulator::read_octets_to_host( void )
{
  std::string ret = dispatch.terminal_to_host;
  dispatch.terminal_to_host.clear();
  return ret;
}

void Emulator::execute( const Parser::Execute* act )
{
  dispatch.dispatch( CONTROL, act, &fb );
}

void Emulator::print( const Parser::Print* act )
{
  assert( act->char_present );

  wchar_t ch = act->ch;
  int chwidth;

#if WCHAR_MAX <= 0xFFFF
  /* Handle UTF-16 surrogate pairs on 16-bit wchar_t systems */
  const uint32_t chcheck = ch;
  wchar_t high_surrogate_to_append = 0;

  if ( chcheck >= 0xD800 && chcheck <= 0xDBFF ) {
    /* High surrogate - save and wait for low surrogate */
    pending_high_surrogate_terminal = ch;
    return;
  } else if ( chcheck >= 0xDC00 && chcheck <= 0xDFFF ) {
    /* Low surrogate */
    if ( pending_high_surrogate_terminal != 0 ) {
      /* Combine with pending high surrogate */
      uint32_t codepoint = ( ( pending_high_surrogate_terminal & 0x3FF ) << 10 |
                             ( ch & 0x3FF ) ) + 0x10000;
      high_surrogate_to_append = pending_high_surrogate_terminal;
      pending_high_surrogate_terminal = 0;
      chwidth = wcwidth_codepoint( codepoint );
    } else {
      /* Orphan low surrogate - treat as unprintable */
      chwidth = -1;
    }
  } else {
    /* Regular character - check for orphan high surrogate */
    if ( pending_high_surrogate_terminal != 0 ) {
      /* Orphan high surrogate - ignore it */
      pending_high_surrogate_terminal = 0;
    }
    chwidth = ch == L'\0' ? -1 : ( Cell::isprint_iso8859_1( ch ) ? 1 : wcwidth( ch ) );
  }
#else
  wchar_t high_surrogate_to_append = 0;
  /*
   * Check for printing ISO 8859-1 first, it's a cheap way to detect
   * some common narrow characters.
   */
  chwidth = ch == L'\0' ? -1 : ( Cell::isprint_iso8859_1( ch ) ? 1 : wcwidth( ch ) );
#endif

  Cell* this_cell = fb.get_mutable_cell();

  switch ( chwidth ) {
    case 1: /* normal character */
    case 2: /* wide character */
      if ( fb.ds.auto_wrap_mode && fb.ds.next_print_will_wrap ) {
        fb.get_mutable_row( -1 )->set_wrap( true );
        fb.ds.move_col( 0 );
        fb.move_rows_autoscroll( 1 );
        this_cell = NULL;
      } else if ( fb.ds.auto_wrap_mode && ( chwidth == 2 )
                  && ( fb.ds.get_cursor_col() == fb.ds.get_width() - 1 ) ) {
        /* wrap 2-cell chars if no room, even without will-wrap flag */
        fb.reset_cell( this_cell );
        fb.get_mutable_row( -1 )->set_wrap( false );
        /* There doesn't seem to be a consistent way to get the
           downstream terminal emulator to set the wrap-around
           copy-and-paste flag on a row that ends with an empty cell
           because a wide char was wrapped to the next line. */
        fb.ds.move_col( 0 );
        fb.move_rows_autoscroll( 1 );
        this_cell = NULL;
      }

      if ( fb.ds.insert_mode ) {
        for ( int i = 0; i < chwidth; i++ ) {
          fb.insert_cell( fb.ds.get_cursor_row(), fb.ds.get_cursor_col() );
        }
        this_cell = NULL;
      }

      if ( !this_cell ) {
        this_cell = fb.get_mutable_cell();
      }

      fb.reset_cell( this_cell );
      /* For surrogate pairs, append both surrogates to form the complete character */
      if ( high_surrogate_to_append != 0 ) {
        this_cell->append( high_surrogate_to_append );
      }
      this_cell->append( ch );
      this_cell->set_wide( chwidth == 2 ); /* chwidth had better be 1 or 2 here */
      fb.apply_renditions_to_cell( this_cell );

      if ( chwidth == 2 && fb.ds.get_cursor_col() + 1 < fb.ds.get_width() ) { /* erase overlapped cell */
        fb.reset_cell( fb.get_mutable_cell( fb.ds.get_cursor_row(), fb.ds.get_cursor_col() + 1 ) );
      }

      fb.ds.move_col( chwidth, true, true );

      break;
    case 0: /* combining character */
    {
      Cell* combining_cell = fb.get_combining_cell(); /* can be null if we were resized */
      if ( combining_cell == NULL ) {                 /* character is now offscreen */
        break;
      }

      if ( combining_cell->empty() ) {
        /* cell starts with combining character */
        /* ... but isn't necessarily the target for a new
           base character [e.g. start of line], if the
           combining character has been cleared with
           a sequence like ED ("J") or EL ("K") */
        assert( !combining_cell->get_wide() );
        combining_cell->set_fallback( true );
        fb.ds.move_col( 1, true, true );
      }
      if ( !combining_cell->full() ) {
        combining_cell->append( ch );
      }
    } break;
    case -1: /* unprintable character - treat as width 1 to stay in sync with terminal */
      /* Most terminals display unassigned codepoints as width 1. If we ignore them,
         cursor position gets out of sync causing rendering artifacts. */
      fb.reset_cell( this_cell );
#if WCHAR_MAX <= 0xFFFF
      if ( high_surrogate_to_append != 0 ) {
        this_cell->append( high_surrogate_to_append );
      }
#endif
      this_cell->append( ch );
      this_cell->set_wide( false );
      fb.apply_renditions_to_cell( this_cell );
      fb.ds.move_col( 1, true, true );
      break;
    default:
      assert( !"unexpected character width from wcwidth()" );
      break;
  }
}

void Emulator::CSI_dispatch( const Parser::CSI_Dispatch* act )
{
  dispatch.dispatch( CSI, act, &fb );
}

void Emulator::OSC_end( const Parser::OSC_End* act )
{
  dispatch.OSC_dispatch( act, &fb );
}

void Emulator::Esc_dispatch( const Parser::Esc_Dispatch* act )
{
  /* handle 7-bit ESC-encoding of C1 control characters */
  if ( ( dispatch.get_dispatch_chars().size() == 0 ) && ( 0x40 <= act->ch ) && ( act->ch <= 0x5F ) ) {
    Parser::Esc_Dispatch act2 = *act;
    act2.ch += 0x40;
    dispatch.dispatch( CONTROL, &act2, &fb );
  } else {
    dispatch.dispatch( ESCAPE, act, &fb );
  }
}

void Emulator::resize( size_t s_width, size_t s_height )
{
  fb.resize( s_width, s_height );
}

bool Emulator::operator==( Emulator const& x ) const
{
  /* dispatcher and user are irrelevant for us */
  return fb == x.fb;
}
