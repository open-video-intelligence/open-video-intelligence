# How to contribute
## Coding Convention
Consistent code conventions are important for several reasons:

- Most importantly: To make it easy to read and understand the code. Remember: you may be the only one writing the code initially, but many people will need to read, understand, modify, and fix the code over its lifetime. These conventions attempt to make this task much easier. If the code consistently follows these standards, it improves developer understanding across the entire source base.
- To attempt to improve code quality through consistency, and requiring patterns that are less likely to result in bugs either initially, or after code modification.

## Code Reviews and PRs
You are encouraged to review incoming PRs; regardless whether you are a committer, a designated reviewer, or just a passer-by.

If you are a committer or a reviewer of the specific component, you are obligated to review incoming related PRs within some reasonable time frame. However, even if you are not a reviewer (designated by committer or submitter), as long as you are in this project, you need to give feedback on PRs especially if you are working on similar topics/components.

The submitter has the first responsibility of keeping the created PR clean and neat (rebase whenever there are merge conflicts), following up the feedback, testing when needed.

Additional requirements for codes
- Each feature should come with a rich set of test cases that can be executed as unit tests during build. If the feature is more invasive or richer, you need more and richer test cases. Refer to other test cases in /tests/unittest directory, which use GTest.
- When new test cases are introduced, the number of new negative test cases should be larger than or equal to the number of new positive test cases.
- For C-code, try to stick with C89.
- For C++-code, try to be compatible with C++17.
- Avoid introducing additional dependencies of libraries.

### Merge Criteria
A PR is required to meet the following criteria.

- It has passed all the unit tests.
- At least ONE committers (reviewers with voting rights) have approved the PR.
   - This is a necessary condition, not sufficient.
   - If the PR touches sensitive codes or may affect wide ranges of components, reviewers will wait for other reviewers to back them up.
   - If the PR is messy, you will need to wait indefinitely to get reviews.
      - Apply general rules of git commits and common senses.
      - Do not write a lengthy commit. Apply a single commit per PR if you are new to the community. Have a single topic per commit. Provide enough background information and references. And so on.
- There is no rejections from any official reviewers.
- There is no pending negative feedbacks (unresolved issues) from reviewers.
- A committer with merging privilege will, then, be able to merge the given PR.
