# Reverse TCP Sock5 Proxy

A tiny Reverse Sock5 Proxy 


## Server

The Server GUI Built with ImGui :V

<img src="https://i.imgur.com/VMs92ep.png">

U can download a prebuilt from [Releases](https://github.com/Coldzer0/ReverseSock5Proxy/releases).
<br>

### Build

You need 
- gcc version 12+ [recommended one for windows](https://github.com/brechtsanders/winlibs_mingw/releases)
- [Conan Package Manager](https://conan.io/)

And run any script from the `scripts` folder, depend on your needs :V

<hr>

## Client/Agent

### Build
```bash
gcc main.c -lws2_32 -O2 -s -o Sock5Agent
```

### TODO
- [ ] Refactor the code to support multi OS
- [X] Finish the Server code
- [X] I don't know

### Resources

- https://www.rfc-editor.org/rfc/rfc1928
- [ImGui](https://github.com/ocornut/imgui)
- [nlohmann JSON](https://github.com/nlohmann/json)
- [mINI](https://github.com/pulzed/mINI)

<hr>

### With ❤️ From Home
