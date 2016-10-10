/*
 * Copyright (C) 2003 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

void	glade_gnome_init			 (void);
void	glade_bonobo_init			 (void);
void	glade_gnome_db_init			 (void);

void	glade_xml_signal_connect_full_hack	(GladeXML*		self,
						 char*			handlername,
						 SCM			func);

void	glade_xml_signal_autoconnect_full_hack	(GladeXML*		self,
						 SCM			func);
