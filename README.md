# runas_system
A tiny but usefull tool for Android SELinux Testing.

This tool is similar to the system provided tool 'run-as', but can let you run as a system, which you can not with 'run-as'.
You can also run as any process by option '-p pid'.

# Usage
The usage is similar to 'run-as'.
I assume you've push the binary in `/data`:

```bash
xxx:# /data/runas_system
```
```bash
xxx:# /data/runas_system -p 1314 id
```

You will need root first, of course.

# Compilation
Compile in AOSP...
