use core::cell::UnsafeCell;
use core::ops::{Deref, DerefMut};
use crate::sync::spinlock::Spinlock;


pub struct Guarded<T> {
    lock: Spinlock,
    data: UnsafeCell<T>,
}

impl<T> Guarded<T> {
    pub const fn new(data: T) -> Self {
        Self {
            lock: Spinlock::new(),
            data: UnsafeCell::new(data),
        }
    }
    
    pub fn lock(&self) -> GuardedGuard<T> {
        self.lock.lock();
        GuardedGuard { guarded: self }
    }
}

pub struct GuardedGuard<'a, T> {
    guarded: &'a Guarded<T>,
}

impl<T> Deref for GuardedGuard<'_, T> {
    type Target = T;
    
    fn deref(&self) -> &T {
        unsafe { &*self.guarded.data.get() }
    }
}

impl<T> DerefMut for GuardedGuard<'_, T> {
    fn deref_mut(&mut self) -> &mut T {
        unsafe { &mut *self.guarded.data.get() }
    }
}

impl<T> Drop for GuardedGuard<'_, T> {
    fn drop(&mut self) {
        self.guarded.lock.unlock();
    }
}


unsafe impl<T> Sync for Guarded<T> {}