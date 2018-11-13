/*
 * Copyright (c) 2011-2017 by Lawrence Livermore National Security, LLC.
 * LLNL-CODE-645430
 *
 * Produced at Lawrence Livermore National Laboratory.
 * Written by  Marty McFadden, mcfadden8@llnl.gov
 *             Kathleen Shoga, shoga1@llnl.gov
 *             Barry Rountree rountree@llnl.gov
 *
 * All rights reserved.
 *
 * This file is part of msr-safe.
 *
 * msr-safe is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * msr-safe is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with msr-safe. If not, see <http://www.gnu.org/licenses/>.
 *
 * This material is based upon work supported by the U.S. Department of
 * Energy's Lawrence Livermore National Laboratory. Office of Science, under
 * Award number DE-AC52-07NA27344.
 */

#ifndef MSR_BATCH_HEADER_INCLUDE
#define MSR_BATCH_HEADER_INCLUDE

void msrbatch_cleanup(int majordev);

int msrbatch_init(int *majordev);

#endif
