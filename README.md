# Starter project code and documention for the BYU Robotics Team

### Basic ROS 2 Commands
```bash
# For creating a package with C++
ros2 pkg create --build-type ament_cmake --license Apache-2.0 --node-name my_node my_package 

# For creating a package with Python
ros2 pkg create --build-type ament_python --license Apache-2.0 --node-name my_node my_package

# These create either a .cpp or .py file inside ./src/<package_name> (see generic file structure) that is where the actual code goes
```
```bash
# To build a package return to the root of the workspace
cd ~/ros2_ws

# Build all packages
colcon build --symlink-install

# Build a specific package
colcon build <package-name> --symlink-install

# If running for the first time in a new terminal run this
source install/local_setup.bash
```

```bash
# To run a package
ros2 run my_package my_node
```

### Building the codebase
Inside the main program, we want to run several packages at the same time and can run them all using the launch package
```bash
# To run the launch file
ros2 launch launcher launch.py
```

### Generic File Structure

```text
.
├── README.md
├── .gitignore
└── src
    ├── package_cpp
    ├── LICENSE
    │   ├── CMakeLists.txt
    │   ├── include
    │   │   └── package_cpp
    │   ├── package.xml
    │   └── src
    │       └── c_node.cpp        <--- This is where you will code for C++
    └── package_py
        ├── LICENSE
        ├── package_py
        │   ├── __init__.py
        │   └── p_node.py      <--- This is where you will code for Python
        ├── package.xml
        ├── resource
        │   └── package_py 
        ├── setup.cfg
        ├── setup.py
        └── test
            ├── test_copyright.py
            ├── test_flake8.py
            └── test_pep257.py

```
