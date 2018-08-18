# Overview

Manually generating unit test inputs and expected outputs is tedious.

Most existing automatic testing provides limited checks against the output.

For math libraries, the correctness of the output is important and not easily reduced to a boolean.

For these types of projects, there are often different existing projects that have already implemented certain features with some limitations.
We can use the existing projects for regression testing against certain subsets of the target project.

Given the passing regression results, the unit test stored and reused to test correctness.

# Components

- Generate and record expected input/output from validation program (Retro)

- Testcase Database/Filesharing Server (https://github.com/raggledodo/dora)

- Extract expected input/output for testing (Antero)
