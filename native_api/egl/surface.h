// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NATIVE_API_EGL_SURFACE_H_
#define NATIVE_API_EGL_SURFACE_H_

#include <EGL/egl.h>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "gpu/ipc/common/surface_handle.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"

namespace egl {
class Config;
class NSWindowBridge;

class Surface : public base::RefCountedThreadSafe<Surface> {
 public:
  Surface(const Config* config, gfx::Size size);
  Surface(const Config* config, EGLNativeWindowType win);

  void set_is_current_in_some_thread(bool flag) {
    is_current_in_some_thread_ = flag;
  }
  bool is_current_in_some_thread() const { return is_current_in_some_thread_; }
  const Config* config() const;

  bool is_offscreen() const;
  gpu::SurfaceHandle window() const;
  const gfx::Size size();
  const gfx::Rect bounds();
  bool is_size_dirty() const { return size_dirty_; }
  void set_size_dirty(bool dirty) { size_dirty_ = dirty; }

  float scale_factor();

  void set_bounds(gfx::Rect bounds);

  static bool ValidatePbufferAttributeList(const EGLint* attrib_list);
  static bool ValidateWindowAttributeList(const EGLint* attrib_list);

#if defined(OS_WIN)
  WNDPROC old_wnd_proc_ = NULL;
#endif

 private:
  void Destroy();

  friend class base::RefCountedThreadSafe<Surface>;
  ~Surface();
  bool size_dirty_ = true;
  bool is_current_in_some_thread_;
  const Config* config_;
  gfx::Rect bounds_;
  EGLNativeWindowType win_;
  bool is_offscreen_;

#if defined(OS_MACOSX)
  NSWindowBridge* window_bridge_;
#endif

  DISALLOW_COPY_AND_ASSIGN(Surface);
};

}  // namespace egl

#endif  // NATIVE_API_EGL_SURFACE_H_
