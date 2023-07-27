# Getting started

You will need conan 2. The compiler profile should specify
`compiler.cppstd=17`. There are conan packages not available in
`conancenter`, you will need to build them yourself. They are available
as recipes, which are located in
[seaops/equipment](https://gitlab.sintef.no/seaops/equipment) under
tools/recipes, and
[cppfmu/cppfmu](https://gitlab.sintef.no/cppfmu/cppfmu). The following
packages should be built: `fmi1`, `fmi2`, `cppfmu`, `fmilibrary`,
`fmu-compliance-checker`, `xtypes`.

``` bash
cd dds-fmu
conan install . -u -b missing
conan build . -c tools.build:skip_test=False
# Repeat previous command, because the test currently fails
```

The created FMU is located in `build/Release/fmus/` on Linux.

# DDS FMU Mediator

  - Create a mediator between cosim/FMU and DDS

  - There are several avenues for accomplishing interoperability between
    FMU and DDS
    
    | No. | Option                     | Has been attempted   | Flexibility | Downsides                         |
    | --- | -------------------------- | -------------------- | ----------- | --------------------------------- |
    | 1   | Cosim observer/manipulator | Yes in FKIN project  | Medium      | Changes to libcosim, compilations |
    | 2   | With cosim slave interface | Yes in SFI MOVE      | Medium      | Changes to libcosim, compilations |
    | 3   | As an FMU                  | Some, using Ratatosk | High        | Implementation effort             |
    

  - Option 3 is most sought, since it can be used by any FMU
    co-simulator and does not require source code changes to libcosim

  - Feature requirements
    
      - Given an idl-file, ability to dynamically create
        publishers/subscribers of these at runtime
      - Could be sufficient to run an idl parser for the dds types plus
        appropriate config
      - Configuration file with a mapping from DDS topics and fields to
        FMU inputs/outputs.
      - Should be able to create appropriate publishers/subscribers
        according to config file
      - Auto-generate modelDescription.xml (and ospDescription.xml? No)
        from config file(s)
      - Could use yaml as auxiliary configuration file
      - Should resources/ bundle necessary stuff for reconfiguration, or
        should there be a compilable project?
          - The FMU should be able to repackage itself

  - Suggested approaches for Option 3
    
      - C/C++
          - \[X\] Fundamendals  
            CMake, conan, viproma/cppfmu to create an FMU
          - \[X\] DDS mediation  
            Hand-code an FMU with simple DDS publish/subscribe
          - Configurability  
            Tools for generating modelDescription.xml and FMU
            necessities
              - Signal mapping DDS topic/type to/from FMU input/output
                defined in a yaml file. Note that fast-dds profiles
                probably serves this purpose.
              - Static linking of runtime dependencies if possible (idlc
                static lib?, static libddsc?) (cyclone-dds, fast-dds)
                  - we are going for fast-dds
          - Extensible  
            Deal with new IDL-files and new signal mappings. Repackaging
            of FMU needed. Without recompilation.
          - Complications  
            Recompilation is needed unless parsing can be done runtime
          - Consistency  
            UUID of IDL file, config file and modelDescription to ensure
            unique FMU
          - Checking  
            Signal connections should be checked for existence (members
            exists and such)
          - YAML  
            Use yaml instead of xml for human input, use and bundle yq
            tool if needed (\!)
      - Python
          - Use a python fmu library, unifmu, pyfmu, pythonfmu
          - Use Cyclonedds-python for DDS side
          - Perhaps PyOxidizer for packaging
          - May be easier to handle dynamic types at runtime
          - How to parse IDL?
          - There are several inconveniences too, especially related to
            packaging
              - python runtime and transitive dependencies, bundled
                appropriately

## Implementation tasks

  - \[X\] Remove temporary Trig pub sub compiled types from dds-fmu

  - \[X\] Add Dynamic Publisher|Subscriber to dds-fmu

  - \[ \] Define real reference structure to map between dynamic types
    for dds/idl and fmu

  - \[X\] Load idl files

  - \[ \] Basic dynamic real publisher
    
      - \[ \] Iterate xtypes to populate fmu structures
      - \[ \] Use dynamic publisher to load idl types and publish on fmu

  - \[4/6\] Load xml profiles as common for Fast-DDS (how many
    participants?)
    
      - \[X\] Load participant profiles (one for pub and one for sub)
      - \[X\] Load publisher profile
      - \[X\] Load subscriber profile
      - \[X\] Load topic profiles (topic names and their qos), if they
        exist
      - \[ \] If the qos is different -\> separate participant is
        needed?
      - \[-\] Load dynamic type profiles (TO BE REPLACED with those
        loaded from idl)
          - \[X\] Use XML first
          - \[ \] Use converter to populate instead
          - \[ \] Use converter to register types instead of xml-based

  - \[ \] Create mapping from dds topic primitive types to fmu value
    references

  - \[-\] Generate modelDescription from mapping
    
      - \[X\] version, guid, CoSimulation
      - \[ \] ModelVariables auto-populate
      - \[ \] ModelStructure auto-populate

  - \[ \] Create publishers/subscribers from mapping

  - \[ \] Add GTest for systematic tests of functionality

  - \[ \] Run valgrind on functionality

  - \[X\] File loader with staging directory (std::filesystem)

  - \[X\] GUID functionality (replicate CMake target) with stduuid
    
      - hard-code which files to hash
      - do not hash line with hash in model description
      - model description contains a version tag, which is acquired from
        API compile def/configure<sub>file</sub> (version.txt)
      - hash all idl files in idl folder (resource) \*.idl
      - hash fast-dds config file(s) (resource) dds-fmu-profile.xml
      - eval GUID must be possible given model description and mentioned
        files

  - \[X\] CMake Target to generate GUID using stduuid functionality

  - \[ \] Generate modelDescription.xml
    
      - given idl files and xml, model description must be generate-able
      - must be possible through bundled library API

  - \[-\] Zip staging directory
    
      - \[X\] zip the staged/binary and generated files
      - \[ \] Figure out why permissions are lost
      - the only file not explicitly available before rendering is
        modelDescription.xml

  - \[ \] Make documentation target and bundle it with the fmu

  - \[ \] Add editorconfig and clang-format to unify formatting

  - \[ \] Figure out to properly handle fast-dds log (and xtypes) in
    conjunction with cppfmu

  - \[3/5\] Repackaging
    
    1.  \[X\] Command line tool: positional arg: point to folder to be
        packaged
    2.  \[ \] Parse idls, xml profile and other config files
          - \[ \] Create mock-up parser with placeholders for each step
    3.  \[X\] Produce GUID based on modelDescription template info and
        config files
          - Allow tweaking e.g. ModelStructure as long as consistency is
            retained?
    4.  \[ \] Write modelDescription from XML tree and evaluated GUID
    5.  \[X\] Zip files into fmu

## Description of signal mapping

  - Let T(type) := {R(eal), I(nteger), B(oolean), S(tring)} be the set
    of FMI reference types

  - V<sub>r,t</sub> is a value reference (uint<sub>32</sub> :=
    U<sub>32</sub>), which is unique for each type t in T

  - For each value reference, there exist a value f(V<sub>r,t</sub>) =
    y<sub>t</sub> in t.

  - y<sub>t</sub> is a mapping from a value reference to a value of type
    t.

  - For each instance of \<fmu<sub>in</sub>, fmu<sub>out</sub>\>:
    
      - Acquire the DynamicData type and iterate through its primitive
        types (leaf nodes)
      - Auxiliary information known is:
          - The iterated DynamicData type and its
            Readable|WritableDynamicDataRef instance
          - All info provided by DynamicData (type, member index?)
          - A way to generate structured name ./\[i\] notation
      - For each leaf node (l) with type P:
          - There is a mapping \(P \to T \cup\, U(\text{nsupported})\)
            that dictates the appropriate FMI setter/getter for which a
            node is to be associated.
          - The mapping P to T may require a cast from one primitive
            type to another.
          - In some cases the mapping from P is in U(nsupported). These
            primitive types are not allowed and will raise runtime error
            during setup.
          - If Leaf node in fmu<sub>in</sub>, dds<sub>out</sub>:
              - Calls FMU Setters (SetT): FMU to DDS (set data from FMU,
                write to DDS)
              - DDS to publish data
              - The value pointed to by y(V<sub>r,t</sub>) is to be set
                on the DDS primitive type, which later will be sent.
              - We need a way to set correct DDS member given
                V<sub>r,t</sub> and y<sub>t</sub> as input.
              - **Note**: It is practical to work with XTypes
                DynamicType instances instead, which can be converted to
                corresponding DDS type just before publishing.
              - Let v<sub>i,t</sub>(x,y) ∈ U<sub>32</sub> x t -\> Ø be a
                visitor writer function for type t.
                  - It takes a V<sub>r,t</sub>, y<sub>t</sub> and writes
                    to the right dynamic type member of type P
                  - We bind information so that the arguments and return
                    types are known at compile-time
              - Increment V<sub>r,t</sub> once a new v<sub>i,t</sub> has
                been defined
          - If Leaf node in fmu<sub>out</sub>, dds<sub>in</sub>:
              - Calls FMU Getters (GetT): DDS to FMU (read from DDS, put
                to FMU)
              - DDS to subscribe data
              - From the received DDS type instance, the correct member
                shall be retrieved and set on the value pointed to by
                value reference V<sub>r,t</sub>.
              - Also here it is practical to work with XTypes
                DynamicType instances. Once read from DDS, convert to
                XTypes dynamic type
              - Let v<sub>o,t</sub>(x) ∈ U<sub>32</sub> -\> t be a
                visitor reader function for type t.
                  - It takes a V<sub>r,t</sub> and returns the right
                    dynamic type member member of type t
                  - We bind information so that the arguments and return
                    types are known at compile-time

## User configuration insight

  - A mapping from FMU signals to DDS signals is to be made possible
  - Knowledge of both FMU signals and DDS types\&topics to be
    interconnected is assumed
  - The user writes configuration files to generate necessary config
    files for both FMU and DDS
      - The IDL file parsed by xtypes is used to convert/generate
        DynamicData types in fast-dds -\> i.e. no idl compiler needed
          - Need to confirm that this is possible. It is, with some
            limitations to annotations.
          - Alternatively, the IDL must be compiled into a dynamic
            library with type definitions that can be loaded at runtime
            (fallback)
  - The user writes XML profiles for DDS-related configuration
      - General configuration of participant, etc.
      - Settings for publishers and subscribers, including topic name,
        data type, qos (esp. durability and reliability)
  - The user or some program writes a mapping between FMU signals and
    DDS topic members
      - The provided information must be sufficient so that a mapping
        between primitive types are possible
      - FMU source type may not be the same as DDS destination primitive
        type
      - Must be possible to construct nested topic member variable names
        to create function mapping from fmu signal
      - Data types in DDS dictates the most closely related type on the
        FMI side
      - There are conventions on with `.` and `[]` for referencing
        non-primitive types
          - It is possible to auto-generate FMU inputs and outputs based
            on DDS topics and their types

## Research notes

  - Given an IDL-file, convert to xtypes, achieved with eprosima/xtypes
    header only library
  - [This converts from xtypes to Dynamic
    Types](https://github.com/eProsima/FastDDS-SH/blob/main/src/Conversion.hpp)
    in Fast DDS -\> perhaps useful
  - [This issue indicates conversion of xtypes to Dynamic
    Types](https://github.com/eProsima/xtypes/issues/82#issuecomment-785089279)
      - \[X\] Investigate if xtypes has been integrated properly, of if
        this conversion is still needed. NEEDED
      - \[X\] If not integrated: load IDL file with xtypes library,
        convert to Dynamic Types
      - The mapping from FMU input/output to DDS publish/subscribe still
        need the information provided by the xtypes in order to
        reference these.
  - [It seems fast dds does not support
    `@optional`?](https://github.com/eProsima/Fast-DDS-Gen/issues/63)
  - \[X\] Specification of subscribers and publishers using Fast-DDS XML
    profiles
      - Important to retain configurability of the profiles made
        possible with the XML files
      - We will not use dynamic types from xml, since it is redundant
        with idl and xtypes with converter to dds

# References

  - [Multicast within
    kubernetes](https://www.spectric.com/post/multicast-within-kubernetes)
  - [k3s](https://k3s.io/)
