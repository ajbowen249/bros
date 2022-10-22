export const Version = 1;
export const NameMaxLength = 12;
export const MaxExtensionLength = 4;
export const DefaultDiskVolumeSize = 4096;
export const HeaderSize = 8;
export const EntrySize = 20;
export const MaxEntries = 50;
export const BlockDataSize = 12;
export const BlockHeaderSize = 4;
export const RootFolder = 255;
export const BlockRecordSize = BlockDataSize + BlockHeaderSize;

const availableBlockSpace = DefaultDiskVolumeSize - HeaderSize - (MaxEntries * EntrySize);
export const MaxBlocks = availableBlockSpace / BlockRecordSize;
(window as any).MaxBlocks = MaxBlocks;
export const VolumeDataCapacity = MaxBlocks * BlockDataSize;

export class FileNameTooLongError extends Error {
    public readonly offendingName: string;
    constructor(offendingName: string) {
        super(`File name too long (${offendingName})`);
        this.offendingName = offendingName;
    }
}

export class FileExtensionTooLongError extends Error {
    public readonly offendingExtension: string;
    constructor(offendingExtension: string) {
        super(`File extension too long (${offendingExtension})`);
        this.offendingExtension = offendingExtension;
    }
}

export class FileSystemFullError extends Error {
    constructor() {
        super('File table is full');
    }
}

export class FileSystemOutOfSpaceError extends Error {
    constructor() {
        super('Out of disk space');
    }
}

export class FileConflictError extends Error {
    public readonly addingEntry: Entry;
    public readonly conflictingEntry: Entry;
    constructor(addingEntry: Entry, conflictingEntry: Entry) {
        super(`Conflicting entries ${addingEntry.name.toString()}`);

        this.addingEntry = addingEntry;
        this.conflictingEntry = conflictingEntry;
    }
}

export class Name {
    private _name: string;
    private _extension: string;

    constructor(name: string, extension: string) {
        this.validateLength(name);
        this.validateLength(extension, false);
        this._name = name;
        this._extension = extension;
    }

    get name(): string { return this._name; }
    set name(newName: string) {
        this.validateLength(newName);
        this._name = newName;
    }

    get extension(): string { return this._extension; }
    set extension(newExtension: string) {
        this.validateLength(newExtension, false);
        this._extension = newExtension;
    }

    toString(): string {
        return `${this.name}${this.extension.length ? `.${this.extension}`  : ''}`;
    }

    equals(other: Name) {
        return other.name === this.name && other.extension === this.extension;
    }

    private validateLength(str: string, isName: boolean = true) {
        const limit = isName ? NameMaxLength : MaxExtensionLength;
        if (str.length > limit) {
            throw new (isName ? FileNameTooLongError : FileExtensionTooLongError)(str);
        }
    }
}

export enum EntryType {
    None = 0,
    Folder = 1,
    File = 2,
}

export class Entry {
    entryType: EntryType;
    parentFolder: FolderEntry | null;
    name: Name;

    constructor(
        entryType: EntryType = EntryType.None,
        name: Name = new Name('', ''),
        parentFolder?: FolderEntry,
    ) {
        this.entryType = entryType;
        this.name = name;
        this.parentFolder = parentFolder ?? null;
    }
}

export class FolderEntry extends Entry {
    placeholder: number;
    constructor(name: Name, parentFolder?: FolderEntry) {
        super(EntryType.Folder, name, parentFolder);
        this.placeholder = 0;
    }
}

export class FileEntry extends Entry {
    data: Uint8Array;
    constructor(name: Name, parentFolder?: FolderEntry, data?: Uint8Array) {
        super(EntryType.File, name, parentFolder);
        this.data = data ?? new Uint8Array([]);
    }
}

export class FileSystem {
    private table: Entry[];

    constructor() {
        this.table = [];
    }

    addEntry(entry: Entry, overwrite: boolean = false) {
        const conflict = this.table.find(x =>
            x.parentFolder === entry.parentFolder &&
            x.name.equals(entry.name)
        );

        if (conflict) {
            if (
                !overwrite ||
                !(conflict.entryType === EntryType.File && entry.entryType === EntryType.File)
            ) {
                throw new FileConflictError(entry, conflict);
            }

            this.deleteEntry(conflict);
        }

        if (this.table.length >= MaxEntries) {
            throw new FileSystemFullError();
        }

        this.table.push(entry);
    }

    deleteEntry(entry: Entry) {
        this.table = this.table.filter(x => x !== entry);
        if (entry.entryType === EntryType.Folder) {
            this.table = this.table.filter(x => x.parentFolder !== entry);
        }
    }

    getTable(): Entry[] {
        return this.table;
    }
}

export function createTestFileSystem(): FileSystem {
    const fs = new FileSystem();

    const sysFolder = new FolderEntry(new Name('SYS', ''));
    const systemFile = new FileEntry(
        new Name('SYSTEM', 'TXT'),
        sysFolder,
        new TextEncoder().encode('JUST AN EXAMPLE TEXT FILE.')
    );

    const userFolder = new FolderEntry(new Name('USR', ''));
    const docsFolder = new FolderEntry(new Name('DOCUMENTS', ''), userFolder);
    const docFile = new FileEntry(
        new Name('EMPTYDOC', 'TXT'),
        docsFolder,
        new TextEncoder().encode('STORE YOUR DOCUMENTS HERE \n OR WHATEVER')
    );
    const configFolder = new FolderEntry(new Name('CONFIG', ''), userFolder);
    const welcomeFile = new FileEntry(
        new Name('WELCOME', 'TXT'),
        undefined,
        new TextEncoder().encode('WELCOME TO BROS, THE NINTENDO OPERATING SYSTEM!')
    );

    fs.addEntry(sysFolder);
    fs.addEntry(systemFile);
    fs.addEntry(userFolder);
    fs.addEntry(docsFolder);
    fs.addEntry(docFile);
    fs.addEntry(configFolder);
    fs.addEntry(welcomeFile);

    return fs;
}
