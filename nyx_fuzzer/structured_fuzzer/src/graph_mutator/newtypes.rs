#[derive(Debug, Copy, Clone, Eq, PartialEq, Hash)]
#[repr(transparent)]
pub struct PortID(u16);
impl PortID {
    pub fn new(x: u16) -> Self {
        Self(x)
    }
    pub fn next(self) -> Self {
        Self(self.0 + 1)
    }
    pub fn as_u16(self) -> u16 {
        self.0
    }
    pub fn as_usize(self) -> usize {
        self.0 as usize
    }
}
#[derive(Debug, Copy, Clone, Eq, PartialEq, Hash)]
#[repr(transparent)]
pub struct OpIndex(usize);
impl OpIndex {
    pub fn new(x: usize) -> Self {
        Self(x)
    }
    pub fn as_usize(self) -> usize {
        return self.0;
    }
}
#[derive(Debug, Copy, Clone, Eq, PartialEq, Hash)]
#[repr(transparent)]
pub struct NodeTypeID(u16);
impl NodeTypeID {
    pub fn new(x: u16) -> Self {
        Self(x)
    }
    pub fn as_u16(self) -> u16 {
        return self.0;
    }
    pub fn as_usize(self) -> usize {
        return self.0 as usize;
    }
}

#[derive(Debug, Copy, Clone, Eq, PartialEq, Hash)]
#[repr(transparent)]
pub struct AtomicTypeID(usize);
impl AtomicTypeID {
    pub fn new(x: usize) -> Self {
        Self(x)
    }
    pub fn as_usize(self) -> usize {
        return self.0;
    }
}

#[derive(Debug, Copy, Clone, Eq, PartialEq, Hash)]
#[repr(transparent)]
pub struct ValueTypeID(u16);
impl ValueTypeID {
    pub fn new(x: u16) -> Self {
        Self(x)
    }
    pub fn as_usize(self) -> usize {
        return self.0 as usize;
    }
}
#[derive(Debug, Copy, Clone, Eq, PartialEq, Hash)]
#[repr(transparent)]
pub struct ConnectorID(u16);
impl ConnectorID {
    pub fn new(x: u16) -> Self {
        Self(x)
    }
    pub fn next(self) -> Self {
        Self(self.0 + 1)
    }
    pub fn as_u16(self) -> u16 {
        return self.0;
    }
}

#[derive(Debug)]
pub enum GraphError {
    UnknownNodeType(NodeTypeID),
    UnknownValueType(ValueTypeID),
    UnknownDataType(AtomicTypeID),
    UnknownID(ConnectorID),
    MissingInput(),
    InvalidSpecs,
}

#[derive(Eq, PartialEq, Debug, Clone)]
pub struct SrcVal {
    pub id: OpIndex,
    pub port: PortID,
}
impl SrcVal {
    pub fn new(id: OpIndex, port: PortID) -> Self {
        Self { id, port }
    }
}
#[derive(Eq, PartialEq, Debug, Clone)]
pub struct DstVal {
    pub id: OpIndex,
    pub port: PortID,
}
impl DstVal {
    pub fn new(id: OpIndex, port: PortID) -> Self {
        Self { id, port }
    }
}
