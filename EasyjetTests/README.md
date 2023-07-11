Easyjet Tests
=============

This package contains regression tests for easyjet. It's important that you implement tests for your workflow: without them we make no promises about keeping your code running.

Adding tests
------------

If you contribute a new way of running easyjet, we encourage you to add a CI test. These tests will generally download a 10 event DAOD from [a designated repository][ci-files] and run your code.

### Creating a new test file

If your test can already run on one of the files in the test file repository, great, you can skip to the next step.

Otherwise follow [the instructions in `training-dataset-dumper`][make-file] to make a new file, and make a merge request to add it to the test file repository.

### Adding a test

Quite a few tests are already implemented in `bin/easyjet-test` and scheduled in the CI via [`.run-gitlab-ci.yaml`][run-ci]. You should be able to follow the general pattern there to schedule your test. You should see the test run automatically on your fork when you push your branch.

[ci-files]: https://gitlab.cern.ch/easyjet/hh4b-test-files
[run-ci]: ../.gitlab/pipelines/.run-gitlab-ci.yaml
[make-file]: https://training-dataset-dumper.docs.cern.ch/tests/#making-daods-from-aods
