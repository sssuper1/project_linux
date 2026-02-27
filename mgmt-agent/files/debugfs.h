/* Copyright (C) 2010-2017  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _NET_MGMT_MODULE_DEBUGFS_H_
#define _NET_MGMT_MODULE_DEBUGFS_H_


struct net_device;

#define MGMT_MODULE_DEBUGFS_SUBDIR "mgmt_agent"


void mgmt_module_debugfs_init(void);
void mgmt_module_debugfs_destroy(void);



#endif /* _NET_BATMAN_ADV_DEBUGFS_H_ */
