// Copyright (c) 2019 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/osr/osr_host_display_client.h"

#include <utility>

#include "base/memory/shared_memory.h"
#include "components/viz/common/resources/resource_format.h"
#include "components/viz/common/resources/resource_sizes.h"
#include "mojo/public/cpp/system/platform_handle.h"
#include "skia/ext/platform_canvas.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/src/core/SkDevice.h"
#include "ui/gfx/skia_util.h"

#if defined(OS_WIN)
#include "skia/ext/skia_utils_win.h"
#endif

namespace atom {

LayeredWindowUpdater::LayeredWindowUpdater(
    viz::mojom::LayeredWindowUpdaterRequest request,
    OnRWHVPaintPixelsCallback callback)
    : callback_(callback), binding_(this, std::move(request)) {}

LayeredWindowUpdater::~LayeredWindowUpdater() = default;

void LayeredWindowUpdater::SetActive(bool active) {
  active_ = active;
}

const void* LayeredWindowUpdater::GetPixelMemory() const {
  return shared_memory_.memory();
}

SkImageInfo LayeredWindowUpdater::GetPixelInfo() const {
  return pixel_info_;
}

void LayeredWindowUpdater::OnAllocatedSharedMemory(
    const gfx::Size& pixel_size,
    mojo::ScopedSharedBufferHandle scoped_buffer_handle) {
  // Make sure |pixel_size| is sane.
  size_t expected_bytes;
  bool size_result = viz::ResourceSizes::MaybeSizeInBytes(
      pixel_size, viz::ResourceFormat::RGBA_8888, &expected_bytes);
  if (!size_result)
    return;

#if defined(WIN32)
  base::SharedMemoryHandle shm_handle;
  MojoResult unwrap_result = mojo::UnwrapSharedMemoryHandle(
      std::move(scoped_buffer_handle), &shm_handle, nullptr, nullptr);
  if (unwrap_result != MOJO_RESULT_OK)
    return;
  base::WritableSharedMemoryRegion shm =
      base::WritableSharedMemoryRegion::Deserialize(
          base::subtle::PlatformSharedMemoryRegion::Take(
              base::win::ScopedHandle(shm_handle.GetHandle()),
              base::subtle::PlatformSharedMemoryRegion::Mode::kWritable,
              shm_handle.GetSize(), shm_handle.GetGUID()));
#else
  base::WritableSharedMemoryRegion shm =
      mojo::UnwrapWritableSharedMemoryRegion(std::move(scoped_buffer_handle));
  if (!shm.IsValid()) {
    DLOG(ERROR) << "Failed to unwrap shared memory region";
    return;
  }
#endif
  pixel_info_ = SkImageInfo::MakeN32(pixel_size.width(), pixel_size.height(),
                                     kPremul_SkAlphaType);
  shared_memory_ = shm.Map();
  if (!shared_memory_.IsValid()) {
    DLOG(ERROR) << "Failed to map shared memory";
    return;
  }
}

void LayeredWindowUpdater::Draw(const gfx::Rect& damage_rect,
                                DrawCallback draw_callback) {
  if (active_) {
    const void* memory = GetPixelMemory();
    if (memory) {
      callback_.Run(damage_rect, pixel_info_, memory);
    }  // else log warn
  }

  std::move(draw_callback).Run();
}

OffScreenHostDisplayClient::OffScreenHostDisplayClient(
    gfx::AcceleratedWidget widget,
    OnRWHVPaintPixelsCallback callback_pixels,
    OnRWHVPaintBitmapCallback callback_bitmap)
    : viz::HostDisplayClient(widget),
      callback_pixels_(callback_pixels),
      callback_bitmap_(callback_bitmap) {}
OffScreenHostDisplayClient::~OffScreenHostDisplayClient() {}

void OffScreenHostDisplayClient::SetActive(bool active) {
  active_ = active;
  if (layered_window_updater_) {
    layered_window_updater_->SetActive(active_);
  }
}

const void* OffScreenHostDisplayClient::GetPixelMemory() const {
  return layered_window_updater_ ? layered_window_updater_->GetPixelMemory()
                                 : nullptr;
}

SkImageInfo OffScreenHostDisplayClient::GetPixelInfo() const {
  return layered_window_updater_ ? layered_window_updater_->GetPixelInfo()
                                 : SkImageInfo{};
}

void OffScreenHostDisplayClient::IsOffscreen(IsOffscreenCallback callback) {
  std::move(callback).Run(true);
}

void OffScreenHostDisplayClient::CreateLayeredWindowUpdater(
    viz::mojom::LayeredWindowUpdaterRequest request) {
  layered_window_updater_ = std::make_unique<LayeredWindowUpdater>(
      std::move(request), callback_pixels_);
  layered_window_updater_->SetActive(active_);
}

}  // namespace atom
