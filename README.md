# Buffer Test for Defold 1.2.181

HTML5 build - https://aglitchman.github.io/defold-buffer-version-test/

The project aims to stress-test Defold's Buffer API. It contains pre-created 500 meshes (named as "chunks") with unique empty buffers. Every frame `main.script` generates new buffers and updates meshes.

### Notes

The project contains an editor script to generate chunks: right-click on `/main/main.collection` and click "Make Chunks".
