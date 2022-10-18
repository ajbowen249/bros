import * as bfs from './BFSAbstract';

const BlockAddressNone = new Uint8Array(new Uint16Array([0xffff]).buffer);

function uint32ToBytes(num: number) {
    const bytes = new Uint32Array(num);
    return new Uint8Array(bytes);
}

function nameToBytes(name: bfs.Name) {
    const bytes = new Uint8Array(bfs.NameMaxLength + bfs.MaxExtensionLength);
    for (let i = 0; i < bfs.NameMaxLength; i++) {
        const char = i < name.name.length ? name.name.charCodeAt(i) : 0;
        bytes[i] = char;
    }

    for (let i = 0; i < bfs.MaxExtensionLength; i++) {
        const char = i < name.extension.length ? name.extension.charCodeAt(i) : 0;
        bytes[bfs.NameMaxLength + i] = char;
    }

    return bytes;
}

export function ToBinary(fileSystem: bfs.FileSystem) {
    const binary = new Uint8Array(bfs.DefaultDiskVolumeSize);
    const headerString = 'BFS';
    binary[0] = headerString.charCodeAt(0);
    binary[1] = headerString.charCodeAt(1);
    binary[2] = headerString.charCodeAt(2);
    binary[3] = bfs.Version;

    const volumeBytes = uint32ToBytes(bfs.DefaultDiskVolumeSize);
    binary[4] = volumeBytes[0];
    binary[5] = volumeBytes[1];
    binary[6] = volumeBytes[2];
    binary[7] = volumeBytes[3];

    // Save ofets of each file record's block offset field so we can populate it later when building the table
    const BlockAddressOfets: Record<number, number> = {};

    for (let entryI = 0; entryI < bfs.MaxEntries; entryI++) {
        const entry = fileSystem.getTable()[entryI] ?? new bfs.Entry();
        const entryOfet = (entryI * bfs.EntrySize) + 8;
        binary[entryOfet] = entry.entryType;
        let folderIndex = entry.parentFolder ?
            fileSystem.getTable().indexOf(entry.parentFolder) :
            bfs.NoFolder;

        if (folderIndex < 0) {
            folderIndex = bfs.NoFolder;
        }

        binary[entryOfet + 1] = folderIndex;
        const nameBytes = nameToBytes(entry.name);
        for (let nameI = 0; nameI < nameBytes.length; nameI++) {
            binary[entryOfet + nameI + 2] = nameBytes[nameI];
        }

        switch(entry.entryType) {
            case bfs.EntryType.File:
                // We'll come back and populate this later
                // TODO: Actually do that ^
                BlockAddressOfets[entryI] = entryOfet + 18;
                binary[entryOfet + 18] = BlockAddressNone[0];
                binary[entryOfet + 19] = BlockAddressNone[1];
                break;
            default:
                binary[entryOfet + 18] = 0;
                binary[entryOfet + 19] = 0;
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

            const blockAddress = new Uint16Array([blockIndex]);
            binary[BlockAddressOfets[entryI]] = blockAddress[0];
            binary[BlockAddressOfets[entryI] + 1] = blockAddress[1];

            const fileBlocks = Math.ceil(file.data.length / bfs.BlockDataSize);
            if ((bfs.MaxBlocks - blockIndex) < fileBlocks) {
                throw new bfs.FileSystemOutOfSpaceError();
            }

            for (let blockI = 0; blockI < fileBlocks; blockI++) {
                const volumeOffset = blockfieldStart + (bfs.BlockRecordSize * blockIndex);
                binary[volumeOffset] = 0x01;
                binary[volumeOffset + 1] = 0;

                const isEndOfFile = blockI == fileBlocks - 1;
                if (!isEndOfFile) {
                    const nextAddress = new Uint16Array([blockIndex + 1]);
                    binary[volumeOffset + 2] = nextAddress[0];
                    binary[volumeOffset + 3] = nextAddress[1];
                } else {
                    binary[volumeOffset + 2] = BlockAddressNone[0];
                    binary[volumeOffset + 3] = BlockAddressNone[1];
                }

                const blockDataStart = blockI * bfs.BlockDataSize;
                const slice = file.data.slice(blockDataStart, blockDataStart + bfs.BlockDataSize);
                for (let dataI = 0; dataI < bfs.BlockDataSize; dataI++) {
                    binary[volumeOffset + 4 + dataI] = slice[dataI];
                }
                blockIndex++;
            }
        }
    }

    return binary;
}
