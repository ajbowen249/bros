import * as bfs from './BFSAbstract';

const BlockAddressNone = 0xffff;

function nameToBytes(name: bfs.Name) {
    const bytes = new DataView(new ArrayBuffer(bfs.NameMaxLength + bfs.MaxExtensionLength));
    for (let i = 0; i < bfs.NameMaxLength; i++) {
        const char = i < name.name.length ? name.name.charCodeAt(i) : 0;
        bytes.setUint8(i, char);
    }

    for (let i = 0; i < bfs.MaxExtensionLength; i++) {
        const char = i < name.extension.length ? name.extension.charCodeAt(i) : 0;
        bytes.setUint8(bfs.NameMaxLength + i, char);
    }

    return bytes;
}

export function ToBinary(fileSystem: bfs.FileSystem) {
    const binary = new DataView(new ArrayBuffer(bfs.DefaultDiskVolumeSize));
    const headerString = 'BFS';
    binary.setUint8(0, headerString.charCodeAt(0));
    binary.setUint8(1, headerString.charCodeAt(1));
    binary.setUint8(2, headerString.charCodeAt(2));
    binary.setUint8(3, bfs.Version);
    binary.setUint32(4, bfs.DefaultDiskVolumeSize, true);

    for (let entryI = 0; entryI < bfs.MaxEntries; entryI++) {
        const entry = fileSystem.getTable()[entryI] ?? new bfs.Entry();
        const entryOffset = (entryI * bfs.EntrySize) + bfs.HeaderSize;
        binary.setUint8(entryOffset, entry.entryType);
        let folderIndex = entry.parentFolder ?
            fileSystem.getTable().indexOf(entry.parentFolder) :
            bfs.RootFolder;

        if (folderIndex < 0) {
            folderIndex = bfs.RootFolder;
        }

        binary.setUint8(entryOffset + 1, folderIndex);
        const nameBytes = nameToBytes(entry.name);
        for (let nameI = 0; nameI < nameBytes.buffer.byteLength; nameI++) {
            binary.setUint8(entryOffset + nameI + 2, nameBytes.getUint8(nameI));
        }

        // We'll come back and populate this later
        binary.setUint16(entryOffset + 18, BlockAddressNone, true);
    }

    let blockIndex = 0;

    const tableSize = bfs.MaxEntries * bfs.EntrySize;
    const blockfieldStart = bfs.HeaderSize + tableSize;

    for (let entryI = 0; entryI < bfs.MaxEntries; entryI++) {
        const entry = fileSystem.getTable()[entryI];
        if (entry?.entryType !== bfs.EntryType.File) {
            continue;
        }

        const file = entry as bfs.FileEntry;
        if (file.data.length === 0) {
            continue;
        }

        const entryOffset = (entryI * bfs.EntrySize) + bfs.HeaderSize;
        binary.setUint16(entryOffset + 18, blockIndex, true);

        const fileBlocks = Math.ceil(file.data.length / bfs.BlockDataSize);
        if ((bfs.MaxBlocks - blockIndex) < fileBlocks) {
            throw new bfs.FileSystemOutOfSpaceError();
        }

        for (let blockI = 0; blockI < fileBlocks; blockI++) {
            const volumeOffset = blockfieldStart + (bfs.BlockRecordSize * blockIndex);
            binary.setUint8(volumeOffset, 0x01);
            binary.setUint8(volumeOffset + 1, 0);

            const isEndOfFile = blockI == fileBlocks - 1;
            if (!isEndOfFile) {
                binary.setUint16(volumeOffset + 2, blockIndex + 1, true);
            } else {
                binary.setUint16(volumeOffset + 2, BlockAddressNone, true);
            }

            const blockDataStart = blockI * bfs.BlockDataSize;
            const slice = file.data.slice(blockDataStart, blockDataStart + bfs.BlockDataSize);
            for (let dataI = 0; dataI < bfs.BlockDataSize; dataI++) {
                binary.setUint8(volumeOffset + 4 + dataI, slice[dataI]);
            }
            blockIndex++;
        }
    }

    return binary;
}
