/*
 * Copyright (c) 2017 Chris Fallin <cfallin@c1f.net>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Lesser Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "threadpool.h"

#include <stdio.h>

void run() {
  threadpool::Threadpool t(100);

  for (int i = 0; i < 10000; i++) {
    t.Enqueue([=]() { fprintf(stderr, "workitem %d\n", i); });
  }

  for (int i = 0; i < 10000; i++) {
    int result = 0;
    t.EnqueueAndWait([&]() { result = 1; });
    if (result != 1) {
      fprintf(stderr, "result != 1\n");
    }
  }
}

int main() {
  run();

  return 0;
}
