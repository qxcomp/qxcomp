# qlcomp 代理注意事项

## Qt3 陷阱:QCString → QByteArray 隐式转换含尾 NUL(2026-11-17)

Qt3 的 QCString 是 QMemArray<char>,size() 把结尾 '\0' 算在内:
- QCString("f") 隐式转 QByteArray 时 size()==2(多 1 字节 NUL)
- 典型事故:DAV Authorization 头被 NUL 弄脏 → 线上 401 风暴
  (见 anystik/.opencode/plans/qt3-dav-auth-nul-key.md)

### 调用约定
- **base64Encode(const QByteArray&)**:必须传显式长度的 QByteArray;
  Qt3 下字符串用 `QByteArray::duplicate(buf, len)`,二进制同理。
  (base64Decode(std::string) 无此问题——std::string 按构造长度计)
- 新增接受 QByteArray 的公共函数时同样注明;严禁在 Qt3 路径
  依赖 QCString 隐式转换传参。

### 验证
base64Encode 的 Qt3 分支已用 /opt/qt338sh(Qt 3.3.8sh)实测:
RFC4648 7 向量 + decode 回验 + 二进制 roundtrip 全绿(2026-11-17)。

## 其他既定口径
- cJSON/qcjson.h 为全仓唯一份(qlvotube/providers 同源,勿再拷贝)
- compatcore34.h 的 Qt3 分支(qFromUtf8/qbaFromCharptr/base64Decode/base64Encode)
  为 Qt3 兼容口径的参考实现,新公共函数应保持双分支同语义
