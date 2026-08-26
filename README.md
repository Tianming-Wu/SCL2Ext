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
    ├── openssl/                # (planned)
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

## Adding an integration

复制 `integrations/_template/` 并按其中 README 的步骤操作：改占位符、顶层加开关、写 `<SCL2Ext/<name>.hpp>` 与 `src/<name>.cpp`。
