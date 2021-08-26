use rand::distributions::weighted::alias_method::WeightedIndex;
use rand::prelude::*;

pub struct Choices<T> {
    weights: WeightedIndex<usize>,
    options: Vec<T>,
}

impl<T> Choices<T> {
    pub fn new(weights: Vec<usize>, options: Vec<T>) -> Self {
        let weights = WeightedIndex::new(weights).unwrap();
        return Self { weights, options };
    }

    pub fn sample<'a, R: Rng>(&'a self, rng: &mut R) -> &'a T {
        let i = self.weights.sample(rng);
        return &self.options[i];
    }
}
