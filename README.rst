
=================
Radix Prefix Tree
=================

While radix tree implementations are great for efficient storage of strings,
many implementations waste memory for cases where the indexed string occupy a
very small portion of the search space.

For example, most radix trees allocate an array that is the size of the string
alphabet for each node. For an alphabet of alphanumerics (``[a-zA-Z0-9]+``)
this requires space of at least ``62 * sizeof(void*)`` per node.

This implementation uses an ordered, dynamic array to store the child nodes,
which should dirastically reduce wasted memory for radix trees sparsely
populated over large alphabets.

License
=======

Copyright 2012 William Heinbockel

Licensed under the Apache License, Version 2.0 (the "License"); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.
