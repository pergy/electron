// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef SHELL_BROWSER_API_ELECTRON_API_COOKIES_H_
#define SHELL_BROWSER_API_ELECTRON_API_COOKIES_H_

#include <memory>
#include <string>

#include "base/callback_list.h"
#include "gin/handle.h"
#include "net/cookies/canonical_cookie.h"
#include "net/cookies/cookie_change_dispatcher.h"
#include "shell/browser/api/trackable_object.h"
#include "shell/common/promise_util.h"

namespace base {
class DictionaryValue;
}

namespace mate {
class Dictionary;
}

namespace net {
class URLRequestContextGetter;
}

namespace electron {

class ElectronBrowserContext;

namespace api {

class Cookies : public mate::TrackableObject<Cookies> {
 public:
  static gin::Handle<Cookies> Create(v8::Isolate* isolate,
                                     ElectronBrowserContext* browser_context);

  // mate::TrackableObject:
  static void BuildPrototype(v8::Isolate* isolate,
                             v8::Local<v8::FunctionTemplate> prototype);

 protected:
  Cookies(v8::Isolate* isolate, ElectronBrowserContext* browser_context);
  ~Cookies() override;

  v8::Local<v8::Promise> Get(const mate::Dictionary& filter);
  v8::Local<v8::Promise> Set(const base::DictionaryValue& details);
  v8::Local<v8::Promise> Remove(const GURL& url, const std::string& name);
  v8::Local<v8::Promise> FlushStore();

  // CookieChangeNotifier subscription:
  void OnCookieChanged(const net::CookieChangeInfo& change);

 private:
  std::unique_ptr<base::CallbackList<void(
      const net::CookieChangeInfo& change)>::Subscription>
      cookie_change_subscription_;
  scoped_refptr<ElectronBrowserContext> browser_context_;

  DISALLOW_COPY_AND_ASSIGN(Cookies);
};

}  // namespace api

}  // namespace electron

#endif  // SHELL_BROWSER_API_ELECTRON_API_COOKIES_H_
