ISA_PROJECT 🔧💻
A simulation and assembler project developed for the ISA (Instruction Set Architecture) course at Tel Aviv University.

This project consists of:
- A custom assembler written in C that translates `.asm` files to memory input format
- A simulator that executes the memory image, mimicking a simplified RISC architecture
- Input/output trace generation to observe register, memory, and hardware behavior
- Test programs such as a sorting routine to validate the ISA design



📁 Project Structure

```
ISA_PROJECT/
├── assembler code/
│   ├── asm.c                # Assembler source code
│   └── asm.exe              # Assembler executable (ignored from Git)
├── asm functions/
│   └── asmfiles/
│       ├── sort.asm         # Assembly source code
│       ├── sort_memin.txt   # Output from the assembler
├── generalTesting/
│   ├── sort_memin.txt       # Input memory for simulator
│   └── [output .txt files]  # (some are ignored to avoid large commits)
├── .gitignore               # File to ignore outputs and binaries
├── README.md                # This file
```


🚀 How to Run

🛠 1. Compile the Assembler

```bash
gcc -o asm "assembler code/asm.c"
```

📜 2. Run the Assembler

```bash
./asm "asm functions/asmfiles/sort.asm"
```

This will generate `sort_memin.txt`.

💻 3. Run the Simulator

```bash
./sim sort_memin.txt diskin.txt irq2in.txt \
      sort_memout.txt sort_regout.txt sort_trace.txt \
      sort_hwregtrace.txt sort_cycles.txt sort_leds.txt \
      sort_display7seg.txt sort_diskout.txt sort_monitor.txt sort_monitor.yuv
```



🧠 Notes

- The file `sort_trace.txt` is too large for GitHub (over 100MB), so it's excluded using `.gitignore`.
- All `.exe` binaries and heavy `.txt` output files are ignored to keep the repo clean.
- You can regenerate any output by rerunning the assembler and simulator.



📦 Requirements

- GCC or Clang
- Make (optional)
- macOS, Linux, or WSL on Windows
- Git



🙋‍♂️ Maintainer

**Aiman Abed**  
[GitHub: @DarkLordF1](https://github.com/DarkLordF1)

Feel free to open issues or pull requests if you're interested in contributing or reporting bugs.

🧾 License

This project is for academic use under Tel Aviv University coursework. Not licensed for commercial distribution.
