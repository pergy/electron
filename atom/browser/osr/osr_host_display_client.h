// Copyright (c) 2019 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_BROWSER_OSR_OSR_HOST_DISPLAY_CLIENT_H_
#define ATOM_BROWSER_OSR_OSR_HOST_DISPLAY_CLIENT_H_

#include <memory>

#include "base/callback.h"
#include "base/memory/shared_memory.h"
#include "components/viz/host/host_display_client.h"
#include "services/viz/privileged/interfaces/compositing/layered_window_updater.mojom.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "ui/gfx/native_widget_types.h"

namespace atom {

typedef base::Callback<void(const gfx::Rect&, const SkImageInfo&, const void*)>
    OnRWHVPaintPixelsCallback;
typedef base::Callback<void(const gfx::Rect&, const SkBitmap&)>
    OnRWHVPaintBitmapCallback;

class LayeredWindowUpdater : public viz::mojom::LayeredWindowUpdater {
 public:
  explicit LayeredWindowUpdater(viz::mojom::LayeredWindowUpdaterRequest request,
                                OnRWHVPaintPixelsCallback callback);
  ~LayeredWindowUpdater() override;

  void SetActive(bool active);
  const void* GetPixelMemory() const;
  SkImageInfo GetPixelInfo() const;

  // viz::mojom::LayeredWindowUpdater implementation.
  void OnAllocatedSharedMemory(
      const gfx::Size& pixel_size,
      mojo::ScopedSharedBufferHandle scoped_buffer_handle) override;
  void Draw(const gfx::Rect& damage_rect, DrawCallback draw_callback) override;

 private:
  OnRWHVPaintPixelsCallback callback_;
  mojo::Binding<viz::mojom::LayeredWindowUpdater> binding_;
  bool active_ = false;
  base::WritableSharedMemoryMapping shared_memory_;
  SkImageInfo pixel_info_;

  DISALLOW_COPY_AND_ASSIGN(LayeredWindowUpdater);
};

class OffScreenHostDisplayClient : public viz::HostDisplayClient {
 public:
  explicit OffScreenHostDisplayClient(
      gfx::AcceleratedWidget widget,
      OnRWHVPaintPixelsCallback callback_pixels,
      OnRWHVPaintBitmapCallback callback_bitmap);
  ~OffScreenHostDisplayClient() override;

  void SetActive(bool active);
  // Theses are accesible from osr_rwhv, but currently unused (bc invalidate
  // requests redraw)
  const void* GetPixelMemory() const;
  SkImageInfo GetPixelInfo() const;

 private:
  void IsOffscreen(IsOffscreenCallback callback) override;

#if defined(OS_MACOSX)
  void OnDisplayReceivedCALayerParams(
      const gfx::CALayerParams& ca_layer_params) override;
#endif

  void CreateLayeredWindowUpdater(
      viz::mojom::LayeredWindowUpdaterRequest request) override;

  std::unique_ptr<LayeredWindowUpdater> layered_window_updater_;
  OnRWHVPaintPixelsCallback callback_pixels_;
  OnRWHVPaintBitmapCallback callback_bitmap_;
  bool active_ = false;

  DISALLOW_COPY_AND_ASSIGN(OffScreenHostDisplayClient);
};

}  // namespace atom

#endif  // ATOM_BROWSER_OSR_OSR_HOST_DISPLAY_CLIENT_H_
