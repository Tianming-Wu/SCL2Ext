# SharedCppLib2 Extension

This project is part of the [SharedCppLib2](https://github.com/Tianming-Wu/SharedCppLib2) ecosystem.


SharedCppLib2 features zero dependency, which means that it cannot introduce any external projects.

So, several features that implementing myself makes no sense goes here. Here are multiple projects that intergrade SharedCppLib2's api system with other projects to make interacting with them even easier inside the SharedCppLib2 ecosystem.

## Structure

```
SCL2Ext/
├── CMakeLists.txt              # 顶层：只做集成开关，不加具体逻辑
└── integrations/
    ├── bit7z/                  # 7-Zip 集成（bit7z）
    ├── openssl/                # OpenSSL 集成（TLS 客户端传输 tls_client）
    └── _template/              # 新集成模板（照抄即可，不会被构建）
```

## Usage

每个集成用三态开关控制（默认 `AUTO`）：

| 值    | 行为 |
|-------|------|
| `AUTO` | 找到外部库就构建，找不到自动跳过（配置不报错） |
| `ON`   | 强制构建，找不到外部库则配置失败 |
| `OFF`  | 跳过 |

```sh
cmake -B build -DSCL2EXT_BIT7Z=ON
```

### bit7z

支持两种获取方式（自动探测，官方优先）：
- 官方配置：`find_package(bit7z)` → target `bit7z::bit7z`
- vcpkg 兼容：`find_package(unofficial-bit7z CONFIG)` → target `unofficial::bit7z::bit7z64`（或 `bit7z32`）

> [!NOTE]
> bit7z 运行时通过 `LoadLibrary` 动态加载 7-Zip 的 DLL（不走导入表），所以最终可执行文件需要把 7-Zip DLL（`7zip::7zip`）复制到可执行文件旁，否则运行时会加载失败。

### openssl

提供 `scl2ext::tls_client` —— 一个基于 OpenSSL 的 TLS 客户端传输，实现了 SharedCppLib2 的 `scl2::transport_interface`，因此可直接作为 HTTP 客户端的底层传输来跑 HTTPS（无需改动协议层）。

```cpp
#include <SCL2Ext/tls.hpp>

scl2ext::tls_client tls;
if (!tls.connect("example.com", 443)) { /* 握手失败 */ }
// 默认启用服务器证书验证（使用系统/OpenSSL 默认信任库）
// 本地测试可关闭：tls.set_verify_peer(false);
const size_t n = tls.write(scl2::bytearray("GET / HTTP/1.1\r\n..."));
scl2::bytearray resp = tls.readAll();
```

- 支持 TLS 1.2+，证书验证默认开启（`set_verify_peer(false)` 可关闭，仅限本地测试）。
- 底层 TCP 用系统 socket（Windows 为 WinSock），平台条件编译，POSIX 亦可用。
- 依赖：`SharedCppLib2::basic`（内含 virtual stream 接口）+ OpenSSL。

## Adding an integration

复制 `integrations/_template/` 并按其中 README 的步骤操作：改占位符、顶层加开关、写 `<SCL2Ext/<name>.hpp>` 与 `src/<name>.cpp`。
