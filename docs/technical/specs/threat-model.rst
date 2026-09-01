============
Threat Model
============

Requirement Mapping
-------------------
Fulfills `[FUNC-001]`, `[FUNC-007]`, `[FUNC-008]`.

STRIDE Analysis
---------------

* **Spoofing**: Strict TDLib session encryption and Telegram OAuth2/QR-code verification prevent identity impersonation.
* **Tampering**: GPG-signed Git tags, SHA-256 object format, and strict compiler hardening flags (`-D_FORTIFY_SOURCE=3`, `-fstack-protector-strong`) protect binary and execution integrity.
* **Information Disclosure**: Zero hardcoded secrets in source control. Sensitive session databases stored in `~/.local/share/grm/` with mode `0700` and optional TDLib database encryption key.
* **Denial of Service**: Asynchronous receiver event loops with timeout guards and graceful signal handling prevent resource starvation and hang states.
* **Elevation of Privilege**: Containerized deployments execute strictly under rootless Podman with non-root UID 10001.
