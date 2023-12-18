# Introduction

-   **dds-fmu main rationale**: Integrate DDS-based software in FMI simulations.
-   **Core feature**: Incorporates DDS signals in FMI simulations in a simple manner.
-   **No need for code compilation**: Just 1) decompress the FMU; 2) configure it; 3) zip it back to an FMU.
-   For **Quick start**: Jump to @ref sec_quickstart.

![img](images/diagram.png "System overview for DDS-FMU.")

OMG DDS @cite omg-dds-2015 is a standard for a data-centric publish and subscribe scheme, which is devised by the Object Management Group (OMG). It defines a set of quality of services (QoS) for data distribution service (DDS). This includes APIs and communication semantics, where real-time publish subscribe (RTPS) @cite omg-dds-rtps-2022 is one such protocol that aims to provide interoperability between DDS implementations from different vendors.

Functional mock-up interface (FMI) @cite fmi-standard-2023 is a widely used standard that defines containers and interfaces for exchanging dynamical simulation models. As defined by the FMI standard, an FMU (Functional Mock-up Unit) is essentially a zip archive that implements the FMI standard for a specific 'system' in form of binaries, code, configuration files, et cetera.

DDS can be used as a communication middleware for components in a software system. These components can consist of graphical user interfaces, hardware instruments, software components, for instance control systems or business logic units. Suppose one such component is a control system module or sensor acquisition module. If the purpose is to use the DDS component together with other FMUs, one needs to somehow translate between the different APIs. One option is to integrate the DDS component directly in an FMU, but this requires access to the software code for proper integration. An alternative, less invasive, approach is to use an adaptation layer that translates between DDS and FMI.

`dds-fmu` implements the latter approach and is inspired by eProsima's Integration Service @cite eprosima-integration-service-2023. `dds-fmu` is **extensible**: the user can create a customised FMU by providing new IDL files and mappings between DDS pub/sub types and FMI inputs/outputs. `dds-fmu` is **configurable**: it comes bundled with all necessary tools to generate `modelDescription.xml`, which is consistent with the custom configuration. `dds-fmu` customisation has no need for a code compiler and no new binary files are created.

# Usage {#sec_quickstart}

A typical workflow can be summarised with these main steps:

-   Change directory: `cd /to/path/with/dds-fmu.fmu`
-   Decompress the FMU: `unzip dds-fmu.fmu`
-   Edit configuration files in `resources/config/`, see details in @ref sec_configuration.
-   [ ] Make bundled tool executable (Linux): `chmod +x resources/tools/linux64/repacker`
-   Create a new FMU: `resources/tools/linux64/repacker create -v -o dds-fmu.fmu /to/path/with/`


## Configuration {#sec_configuration}

Preparing the FMU for a customised setup is done by editing configuration files, which are located in the `resources` folder. The layout of this folder can be seen in the figure below. Typically, a customisation consists of editing three files, which will be elaborated below:

1.  `dds-fmu.idl` Data type specification with IDL files (@ref sec_idl).
2.  `ddsfmu_mapping.xml` DDS-to-FMU mapping (@ref sec_ddsfmu).
3.  `dds_profile.xml` Fast-DDS XML profiles (@ref sec_profiles).

![img](images/resources.svg "Folder structure of the resources directory.")


### Data type specification with IDL {#sec_idl}

The IDL standard @cite omg-idl-2018 is a descriptive way to define data types and more. We make use of it to define data structures on DDS that we want to integrate with FMU. The basic notation is not unlike regular C++ and in the listing below we provide an example, which cover many use cases.

```C
#pragma once

/// A module translates to namespaces in C++
module idl {

  /// An enum with SLEEP=0, OKAY=1 etc.
  enum MyEnum { SLEEP, OKAY, ALERT, FAILURE, DEAD };

  /// A class with member variables
  struct Klass {
    string str;      ///< A std::string
    @key uint8 ui8;  ///< Unsigned integer 8-bit
    int32 i32;       ///< Signed integer 32-bit
    int64 i64;       ///< 8, 16, 32, 64, int and uint, are supported
    double d_val;    ///< Double precision floating point float64
    float f_val;     ///< Single precision floating point float32
    boolean enabled; ///< Boolean
    MyEnum status;   ///< Enum type
    float my_array [10];     ///< 1D array of floats
    uint32 my_matrix [3][2]; ///< 2D array of unsigned integers
  };

};
```

Suppose the contents above are added to `dds-fmu.idl`, the file will be parsed by the FMU, and `idl::Klass` becomes available as a type.


### DDS-to-FMU mapping {#sec_ddsfmu}

The file `ddsfmu_mapping.xml` contains elements that specify which DDS topics to map to FMU inputs and outputs, see figure below. The *topic* attribute is a name identifier for a DDS Topic entity. Each topic is associated with a data *type*, which in our case is defined by our IDL file. Note that **FMU outputs** are **subscribed** DDS signals, and **FMU inputs** are **published** DDS signals. **DDS input = FMU output** and **DDS output = FMU inputs**. The user defines the necessary of FMU inputs and outputs using `<fmu_in>` and `<fmu_out>` elements, respectively. See the listing below for an example. For each element of `<fmu_in>` a DDS DataWriter is created, and likewise, for each `<fmu_out>` a DDS DataReader. The attribute *key_filter* of the `<fmu_out>` node indicates whether the FMU should perform key filtering on the output signals. This is disabled by default, which would result in all data on the topic being processed by the DDS DataReader. If it is enabled, on the other hand, corresponding FMU parameters will be generated in the `modelDescription.xml`.

![img](images/ddsfmu-mapping.svg "`ddsfmu_mapping` XML specification.")

```xml
<ddsfmu>
  <fmu_in topic="ToPublish" type="idl::Klass" />
  <fmu_out topic="ToSubscribe" type="idl::Klass" key_filter="true" />
</ddsfmu>
```

The `repacker` tool generates the `modelDescription.xml` based on this mapping. Suppose the unzipped contents with modified configuration files is located in `/my/custom/fmu`. By running the commands below, the user can inspect the generated `/my/custom/fmu/modelDescription.xml`.

```bash
cd /my/custom/fmu
resources/tools/linux64/repacker generate .
```


### Fast-DDS XML profiles {#sec_profiles}

A user can configure the Fast-DDS to a great extent by means of XML profiles. Central concepts such as domain id, QoS (like durability and reliability), and much more are configured using configuration profiles for various DDS entities. These profiles are loaded by purposefully specifying the `profile_name` attribute for an element type, see the figure below. The profiles for *participant*, *publisher*, and *subscriber* are attempted loaded by `profile_name="dds-fmu-default"`, with fallback to builtin default QoS. Profiles for `topic`, `data_writer`, and `data_reader` elements are attempted loaded by `profile_name="[topic]"`, where *topic* is as defined in <fig:ddsfmu>, with fallback to default QoS. This means that the user can specify custom profiles for specific `topic`, `data_reader`, and `data_writer` entities. XML profile documentation for each DDS entity type can be found on Fast-DDS online documentation @cite eprosima-fast-dds-xml-profiles-2023. The FMU comes with an example `dds_profile.xml`, which can be edited as needed.

![img](images/xml-profiles.svg "`dds_profile` XML layout, where `n_w` is number of data readers and `n_r` is number of data readers.")

Continuing the example from previous sections, it could be necessary to add custom QoS for the `data_writer`. Then, the `dds_profile.xml` would contain an element as indicated below.

```xml
<dds>
  <profiles>
    ...
    <data_writer profile_name="ToPublish">
      <qos>
        <reliability>
          <kind>RELIABLE</kind>
        </reliability>
      </qos>
    </data_writer>
  </profiles>
</dds>
```

# Implementation overview

DDS supports data exchange of user-defined data structures. These are often defined using an interface definition language (IDL), whose grammar is specified by the OMG IDL @cite omg-idl-2018. What the IDL files defines, can be represented as dynamic types through the XTypes API specification @cite omg-dds-xtypes-2020. `dds-fmu` makes use of this standard through a vendor implementation, namely `eProsima xtypes` @cite eprosima-xtypes-2023. Moreover, `dds-fmu` uses `eProsima Fast-DDS` @cite eprosima-fast-dds-2023, which implements DDS RTPS. `dds-fmu` parses IDL files into xtypes DynamicData and, with the help of code taken from @cite eprosima-integration-service-2023, converts between xtypes DynamicData and Fast-DDS DynamicData. As a result, `dds-fmu` supports DDS communication with data types defined in IDL files without the need for code compilation. The xTypes API facilitates access to members of DynamicData in a way that infers the type kind of each member. `dds-fmu` makes use of this feature to ensure that each member is read or write accessed as the appropriate primitive type, as supported from the FMU side. Since `dds-fmu` is a co-simulation FMU, the implementation of the API is achieved with the help of `cppfmu` @cite cppfmu-2023. Currently, `dds-fmu` supports FMI 2.0, which means that there are some limitations in terms of mapping from DynamicData member types to FMI types, see table below for an overview of supported data type mapping.

| Type kind   | FMI 2.0 type | Comment |  | Type kind     | Comment |
|----------- |------------ |------- |--- |------------- |------- |
| boolean     | fmiBoolean   |         |  | long double   | N/A     |
| int8        | fmiInteger   |         |  | char16        | N/A     |
| uint8       | fmiInteger   |         |  | wide char     | N/A     |
| int16       | fmiInteger   |         |  | bitset        | N/A     |
| uint16      | fmiInteger   |         |  | sequence type | N/A     |
| int32       | fmiInteger   |         |  | wstring       | N/A     |
| uint32      | fmiReal      |         |  | map type      | N/A     |
| int64       | fmiReal      | Lossy   |  |               |         |
| uint64      | fmiReal      | Lossy   |  |               |         |
| float       | fmiReal      |         |  |               |         |
| double      | fmiReal      |         |  |               |         |
| string      | fmiString    |         |  |               |         |
| char8       | fmiString    |         |  |               |         |
| enumeration | fmiInteger   |         |  |               |         |


## Data structure demultiplexing and model description

An IDL data structure can be complex, with non-primitive types and nested data structures. These members needs to be demultiplexed in a way that allows the scalar variable access interface of FMI 2.0 to read or write member variables. This must be done in a manner that correctly casts to their primitive type. While parsing a requested DynamicData variable, `dds-fmu` instantiates visitor functions for read and write, with appropriate reference to the DynamicData's primitive type, as well as casting for input and output types. These visitor functions are stored in vectors in such a way that with so-called value references, they can be directly accessed by FMU setters and getters.

`dds-fmu` comes bundled with an executable command line tool for generating `modelDescription.xml`. In short: given `IDL` files, Fast-DDS configuration files, and a DDS-to-FMU mapping specification, the tool automatically generates `modelDescription.xml`. The output model description creates `<ModelVariables>` elements with `<ScalarVariable>` entries, and `<ModelStructure>` element with `<Outputs>`. All the `<ScalarVariables>` entries have attribute `variability=discrete` when they consist solely of inputs and outputs: `causality=input|output`. If there are any `@key` variables, additional entries with `causality=parameter` and `variability=fixed` will be created. The generated `<ScalarVariable>` entries have `name` attribute based on the FMI standard's `structured` variable naming convention. The variable name is constructed as `name=[pubsub].[topic name].[structured name]`, where `topic name` is as prescribed in the DDS-to-FMU mapping specification file, and `pubsub` is `pub` for input and `sub` for output. For `@key` parameters, they will have naming `name=key.sub.[topic name].[structured name]`.


## Configuration of DDS entities and QoS settings

Each instance of `dds-fmu` only creates a single DDS Participant, DDS Publisher, and DDS Subscriber. As a consequence, the QoS settings for these entities will be the same for all DDS DataReaders and DDS DataWriters in the current FMU instance. However, it is possible to specify QoS for each DataReader and DataWriter. The QoS settings for all mentioned entities are set through Fast-DDS XML profiles. These profiles are documented in the [Fast-DDS documentation](https://fast-dds.docs.eprosima.com/en/latest/fastdds/xml_configuration/making_xml_profiles.html) @cite eprosima-fast-dds-xml-profiles-2023. This setup may not suit complex use cases. Then, one approach would entail splitting the DDS mapping into multiple FMUs. For details on how to do profile configuration, see @ref sec_quickstart.

## Data flow

The interaction with DDS reader and writer entities are done in each call to DoStep() on the FMI side. Writing DDS data is done before reading. If the reader QoS is configured to have history greater than one, all data is fetched, but only the latest sample is kept. Effectively, this approach is a sample and hold. See the figure below for a sequence diagram of DoStep().

![img](images/sequence.svg "Sequence of actions in DoStep().")

## Limitations and caveats

There are some things the user should be aware of to avoid unnecessary troubleshooting. Below we list several points and in some cases suggest workarounds.

-   **Lost samples:** Only the last read sample is kept. Samples may therefore be lost, decrease step size.
-   **Old samples lingers:** There is no expiration of sampled data. Frozen signals are not detected.
    - `key_filter` With key filtering this can become particularly evident.
-   **Sending to itself is possible:** The data flow is implemented so that write occurs before read; there will be a sample lag.
-   **Loss of precision:** Some data types cannot easily be represented with available FMI 2.0 types. In such cases, another data type is used, which may lead to loss of precision.
-   **Several FMU instances is conditionally possible:** Do not use multiple `dds-fmu` instances in the simulator instance if they are on the same DDS Domain ID. There are workarounds for some simulators. In the case of `cosim` @cite cosim-2023 you can use `proxyfmu` @cite cosim-2023-proxyfmu on additional `dds-fmu` instances.

## Missing features

-   Allow using preprocessor when parsing IDL files (e.g. use `#include "file.idl"` in the IDL).
-   Sequence types, e.g. `std::vector<TYPE>`
-   Limited support for IDL annotations.
    - `@key` partially supported: primitive types, string and enumerations. This excludes directly on structs, or array-like members.
    - `@optional` and `@id` is supported by the IDL parser via a patch, but ignored by our implementation
-   FMI 3.0 support
