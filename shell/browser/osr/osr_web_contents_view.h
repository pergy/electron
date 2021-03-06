// Copyright (c) 2016 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef SHELL_BROWSER_OSR_OSR_WEB_CONTENTS_VIEW_H_
#define SHELL_BROWSER_OSR_OSR_WEB_CONTENTS_VIEW_H_

#include "shell/browser/api/electron_api_offscreen_window.h"

#include "content/browser/renderer_host/render_view_host_delegate_view.h"  // nogncheck
#include "content/browser/web_contents/web_contents_view.h"  // nogncheck
#include "content/public/browser/web_contents.h"
#include "shell/browser/osr/osr_render_widget_host_view.h"

#if defined(OS_MACOSX)
#ifdef __OBJC__
@class OffScreenView;
#else
class OffScreenView;
#endif
#endif

namespace electron {

class OffScreenWebContentsView
    : public content::WebContentsView,
      public content::RenderViewHostDelegateView,
      public OffScreenRenderWidgetHostView::Initializer,
      public api::OffscreenWindow::Observer {
 public:
  OffScreenWebContentsView(bool transparent,
                           float scale_factor,
                           const OnPaintCallback& callback,
                           const OnTexturePaintCallback& texture_callback);
  ~OffScreenWebContentsView() override;

  void SetWebContents(content::WebContents*);
  void SetOffscreenWindow(api::OffscreenWindow* window);

  // api::OffscreenWindow::Observer:
  void OnWindowResize() override;
  void OnWindowClosed() override;

  // content::WebContentsView:
  gfx::NativeView GetNativeView() const override;
  gfx::NativeView GetContentNativeView() const override;
  gfx::NativeWindow GetTopLevelNativeWindow() const override;
  void GetContainerBounds(gfx::Rect* out) const override;
  void SizeContents(const gfx::Size& size) override;
  void Focus() override;
  void SetInitialFocus() override;
  void StoreFocus() override;
  void RestoreFocus() override;
  void FocusThroughTabTraversal(bool reverse) override;
  content::DropData* GetDropData() const override;
  gfx::Rect GetViewBounds() const override;
  void CreateView(gfx::NativeView context) override;
  content::RenderWidgetHostViewBase* CreateViewForWidget(
      content::RenderWidgetHost* render_widget_host) override;
  content::RenderWidgetHostViewBase* CreateViewForChildWidget(
      content::RenderWidgetHost* render_widget_host) override;
  void SetPageTitle(const base::string16& title) override;
  void RenderViewCreated(content::RenderViewHost* host) override;
  void RenderViewReady() override;
  void RenderViewHostChanged(content::RenderViewHost* old_host,
                             content::RenderViewHost* new_host) override;
  void SetOverscrollControllerEnabled(bool enabled) override;

#if defined(OS_MACOSX)
  bool CloseTabAfterEventTrackingIfNeeded() override;
#endif

  // content::RenderViewHostDelegateView
  void StartDragging(const content::DropData& drop_data,
                     blink::WebDragOperationsMask allowed_ops,
                     const gfx::ImageSkia& image,
                     const gfx::Vector2d& image_offset,
                     const content::DragEventSourceInfo& event_info,
                     content::RenderWidgetHostImpl* source_rwh) override;
  void UpdateDragCursor(blink::WebDragOperation operation) override;

  // OffScreenRenderWidgetHostView::Initializer
  bool IsTransparent() const override;
  const OnPaintCallback& GetPaintCallback() const override;
  const OnTexturePaintCallback& GetTexturePaintCallback() const override;
  gfx::Size GetInitialSize() const override;

  void SetPainting(bool painting);
  bool IsPainting() const;
  void SetFrameRate(int frame_rate);
  int GetFrameRate() const;
  void SetScaleFactor(float scale_factor);
  float GetScaleFactor() const;

 private:
#if defined(OS_MACOSX)
  void PlatformCreate();
  void PlatformDestroy();
#endif

  OffScreenRenderWidgetHostView* GetView() const;

  api::OffscreenWindow* offscreen_window_;

  const bool transparent_;
  float scale_factor_;
  bool painting_ = true;
  int frame_rate_ = 60;
  OnPaintCallback callback_;
  OnTexturePaintCallback texture_callback_;

  // Weak refs.
  content::WebContents* web_contents_ = nullptr;

#if defined(OS_MACOSX)
  OffScreenView* offScreenView_;
#endif
};

}  // namespace electron

#endif  // SHELL_BROWSER_OSR_OSR_WEB_CONTENTS_VIEW_H_
