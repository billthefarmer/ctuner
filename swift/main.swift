//
//  main.swift
//  Tuner
//
//  Created by Bill Farmer on 21/12/2022.
//  Copyright © 2017 Bill Farmer. All rights reserved.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

import Cocoa

autoreleasepool
{
    let delegate = AppDelegate()
    // NSApplication delegate is a weak reference,
    // so we have to make sure it's not deallocated.
    // In Objective-C you would use NS_VALID_UNTIL_END_OF_SCOPE
    withExtendedLifetime(delegate,
    {
	let application = NSApplication.shared
	application.delegate = delegate
	application.run()
	application.delegate = nil
    })
}
