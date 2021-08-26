use std::rc::Rc;
use std::ptr;
use std::hash::{Hash,Hasher};

#[derive(Clone,Debug)]
pub struct HashAsRef<T>(Rc<T>);

impl<T> Hash for HashAsRef<T> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        let ptr: *const T = &*self.0;
        ptr::hash(ptr, state);
    }
}

impl<T> PartialEq for HashAsRef<T> {
    fn eq(&self, other: &Self) -> bool {
        Rc::ptr_eq(&self.0, &other.0)
    }
}
impl<T> Eq for HashAsRef<T> {}

impl<T> HashAsRef<T>{
    pub fn new(rc: Rc<T>) -> Self{
        return Self(rc);
    }
    pub fn into_rc(self) -> Rc<T>{
        self.0
    }
    pub fn as_usize(&self) -> usize{
        let ptr: *const T = &*self.0;
        return ptr as usize;
    }
}