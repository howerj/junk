fn binary_search<T: Ord>(mut array: &[T], key: T) -> Option<T> {
    use std::cmp::Ordering::*;

    while !array.is_empty() {
        let middle = array.len() / 2;
        match key.cmp(&array[middle]) {
            Less => array = &array[..middle],
            Equal => return Some(key),
            Greater => array = &array[middle + 1..],
        }
    }
    None
}
